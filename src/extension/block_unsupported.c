#include "postgres.h"
#include "fmgr.h"
#include "commands/event_trigger.h"
#include "tcop/utility.h"
#include "utils/elog.h"
#include "parser/analyze.h"
#include "nodes/parsenodes.h"

PG_MODULE_MAGIC;

static ProcessUtility_hook_type prev_ProcessUtility = NULL;

PG_FUNCTION_INFO_V1(block_unsupported_statements);

Datum
block_unsupported_statements(PG_FUNCTION_ARGS)
{
    PG_RETURN_VOID();
}

static const struct {
    int number;
    const char *string;
} node_type_to_statement[] = {
    {227, "CREATE VIEW"},
    {195, "TRUNCATE TABLE"},
    {186, "CREATE SEQUENCE"},
    {239, "CREATE MATERIALIZED VIEW"},
    {157, "CREATE TEMPORARY TABLE"},
    {159, "CREATE TABLESPACE"},
    {178, "CREATE TRIGGER"},
    {223, "CREATE TYPE"},
    {229, "CREATE DATABASE"},
    {163, "CREATE EXTENSION"},
    {234, "ALTER SYSTEM"}
};


/* Hook function implementation */
static void block_unsupported_statements_hook(PlannedStmt *pstmt, const char *queryString,
                                      bool isTopLevel, ProcessUtilityContext context, 
                                      ParamListInfo params, QueryEnvironment *queryEnv,
                                      DestReceiver *dest, QueryCompletion *completionTag)
{
    Node *parsetree = pstmt->utilityStmt;

    // ereport(NOTICE,
    //         (errmsg("Hook called for query: %s", queryString)));

    // ereport(NOTICE,
    //         (errmsg("Node type: %d", nodeTag(parsetree))));

    if (IsA(parsetree, CreateStmt))
    {
        CreateStmt *stmt = (CreateStmt *)parsetree;
        ereport(NOTICE,
                (errmsg("Processing CREATE TABLE statement")));
        
        if (stmt->constraints != NIL)
        {
            ereport(ERROR,
                    (errmsg("CREATE TABLE with constraints is unsupported")));
            return;
        }
    }

    for (int i = 0; i < sizeof(node_type_to_statement) / sizeof(node_type_to_statement[0]); i++) {
        if (nodeTag(parsetree) == node_type_to_statement[i].number) {
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
