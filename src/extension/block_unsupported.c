#include "postgres.h"
#include "fmgr.h"
#include "commands/event_trigger.h"
#include "tcop/utility.h"
#include "utils/elog.h"
#include "parser/analyze.h"
#include "nodes/parsenodes.h"
#include "nodes/pg_list.h" // Include this header for the foreach macro
#include "executor/spi.h"

PG_MODULE_MAGIC;

static ProcessUtility_hook_type prev_ProcessUtility = NULL;

PG_FUNCTION_INFO_V1(block_unsupported_statements);

static bool table_has_data(const char *table_name);

Datum block_unsupported_statements(PG_FUNCTION_ARGS)
{
    PG_RETURN_VOID();
}

static const struct
{
    NodeTag tag;
    const char *string;
} node_type_to_statement[] = {
    {T_ViewStmt, "CREATE VIEW"},
    {T_TruncateStmt, "TRUNCATE TABLE"},
    {T_AlterSystemStmt, "ALTER SYSTEM"},
    {T_CreateExtensionStmt, "CREATE EXTENSION"},
    {T_CreateSeqStmt, "CREATE SEQUENCE"},
    {T_CreateTableAsStmt, "CREATE MATERIALIZED VIEW or CREATE TABLE AS"},
    {T_CreateTrigStmt, "CREATE TRIGGER"},
    {T_CreateTableSpaceStmt, "CREATE TABLESPACE"},
    // <begin> CREATE TYPE
    {T_CompositeTypeStmt, "CREATE TYPE (composite types)"},
    {T_CreateRangeStmt, "CREATE TYPE (range types)"},
    {T_CreateEnumStmt, "CREATE TYPE (enum types)"},
    // <end> CREATE TYPE
    {T_CreatedbStmt, "CREATE DATABASE"},
    {T_VacuumStmt, "VACUUM"},
};

/* Hook function implementation */
static void block_unsupported_statements_hook(PlannedStmt *pstmt, const char *queryString,
                                              bool isTopLevel, ProcessUtilityContext context,
                                              ParamListInfo params, QueryEnvironment *queryEnv,
                                              DestReceiver *dest, QueryCompletion *completionTag)
{
    Node *parsetree = pstmt->utilityStmt;

    ereport(NOTICE,
            (errmsg("Hook called for query: %s", queryString)));

    ereport(NOTICE,
            (errmsg("Node type: %d", nodeTag(parsetree))));

    // Disallow create temporary table
    if (IsA(parsetree, CreateStmt))
    {
        CreateStmt *stmt = (CreateStmt *)parsetree;
        if (stmt->relation->relpersistence == 't')
        {
            ereport(ERROR,
                    (errmsg("CREATE TEMPORARY TABLE is unsupported")));
            return;
        }

        if (stmt->inhRelations != NIL)
        {
            ereport(ERROR,
                    (errmsg("CREATE TABLE INHERITS is unsupported")));
            return;
        }

        if (stmt->partspec != NULL) // Change from NIL to NULL for proper pointer comparison
        {
            ereport(ERROR,
                    (errmsg("CREATE TABLE PARTITION is unsupported")));
            return;
        }

        ListCell *cell;
        foreach (cell, stmt->tableElts)
        {
            ColumnDef *col = (ColumnDef *)lfirst(cell);
            if (col->collClause != NIL && col->collClause->collname != NIL) // Check for collation
            {
                ereport(ERROR,
                        (errmsg("CREATE TABLE COLLATE is unsupported")));
                return;
            }
        }
    }

    // Disallow sorted index creation
    if (IsA(parsetree, IndexStmt))
    {
        IndexStmt *stmt = (IndexStmt *)parsetree;

        if (stmt->indexParams != NIL)
        {
            ListCell *cell;
            foreach (cell, stmt->indexParams)
            {
                IndexElem *indexElem = (IndexElem *)lfirst(cell);
                if (indexElem->ordering == SORTBY_ASC || indexElem->ordering == SORTBY_DESC)
                {
                    ereport(ERROR,
                            (errmsg("CREATE Index with ordering ASC or DESC is unsupported")));
                    return;
                }
            }
        }

        if (stmt->indexIncludingParams != NIL)
        {
            ListCell *cell;
            foreach (cell, stmt->indexIncludingParams)
            {
                IndexElem *indexElem = (IndexElem *)lfirst(cell);
                if (indexElem->ordering == SORTBY_ASC || indexElem->ordering == SORTBY_DESC)
                {
                    ereport(ERROR,
                            (errmsg("CREATE Index with ordering ASC or DESC is unsupported")));
                    return;
                }
            }
        }

        char *table_name = stmt->relation->relname;

        /* Check if the table has data */
        if (table_has_data(table_name))
        {
            ereport(ERROR,
                    (errmsg("Cannot create index on table \"%s\" because it contains data.", table_name)));
        }
    }

    // Disallow CREATE FUNCTION with languages other than sql
    if (IsA(parsetree, CreateFunctionStmt))
    {
        CreateFunctionStmt *stmt = (CreateFunctionStmt *)parsetree;
        ListCell *cell;
        foreach (cell, stmt->options)
        {
            DefElem *defel = (DefElem *)lfirst(cell);
            if (strcmp(defel->defname, "language") == 0)
            {
                char *language_arg = defel->arg ? nodeToString(defel->arg) : "empty";
                ereport(NOTICE,
                        (errmsg("Hook called for query: %s", language_arg)));
                if (strcmp(language_arg, "\"sql\"") != 0)
                {
                    ereport(ERROR,
                            (errmsg("CREATE FUNCTION with language %s not supported", language_arg)));
                    return;
                }
            }
        }
    }

    // Checks for all other unsupported nodes from node_type_to_statement map
    NodeTag parsetreeTag = nodeTag(parsetree);
    for (int i = 0; i < sizeof(node_type_to_statement) / sizeof(node_type_to_statement[0]); i++)
    {
        if (parsetreeTag == node_type_to_statement[i].tag)
        {
            ereport(ERROR,
                    (errmsg("%s statements are unsupported", node_type_to_statement[i].string)));
            return;
        }
    }

    /* Only proceed with utility processing if the statement is allowed */
    if (prev_ProcessUtility)
        prev_ProcessUtility(pstmt, queryString, isTopLevel, context, params, queryEnv, dest, completionTag);
    else
        standard_ProcessUtility(pstmt, queryString, isTopLevel, context, params, queryEnv, dest, completionTag);
}

/* Extension initialization */
void _PG_init(void)
{
    prev_ProcessUtility = ProcessUtility_hook;
    ProcessUtility_hook = block_unsupported_statements_hook;
}

/* Extension cleanup */
void _PG_fini(void)
{
    ProcessUtility_hook = prev_ProcessUtility;
}

static bool table_has_data(const char *table_name)
{
    int ret;
    bool result = false;
    char query[256];

    /* Connect to the SPI manager */
    SPI_connect();

    /* Build and execute the query to check row count */
    snprintf(query, sizeof(query), "SELECT 1 FROM %s LIMIT 1", table_name);
    ret = SPI_execute(query, true, 1);

    if (ret == SPI_OK_SELECT && SPI_processed > 0)
        result = true;

    /* Disconnect from SPI */
    SPI_finish();

    return result;
}