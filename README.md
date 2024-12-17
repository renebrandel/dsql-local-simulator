# DSQL Local Simulator (**un**official)

This is a **PROTOTYPE** PostgreSQL Docker container hosted on port 5432 that simulates the SQL expression support matrix found in [Amazon Aurora DSQL](https://aws.amazon.com/rds/aurora/dsql/).

This is an unofficial simulator and IS NOT 100% equivalent to the actual live Amazon Aurora DSQL service. The list of unsupported features are based on the following [official documentation](https://docs.aws.amazon.com/aurora-dsql/latest/userguide/working-with-postgresql-compatibility-unsupported-features.html).

## How to get started

Pre-requisites:
- [Docker](https://docker.com)

Instructions:
1. Run `./build-and-run.sh`
  - This should build, run the PostgreSQL database, and connect the `psql` CLI to the database.
2. Run the following SQL statement:
```sql
DROP EXTENSION IF EXISTS block_unsupported;
CREATE EXTENSION block_unsupported;
```
3. Try an unsupported statement and see it fail.

Example failing statements:
ALTER SYSTEM
```sql
ALTER SYSTEM SET random_page_cost = '1.1';
```

CREATE TYPE
```sql
CREATE TYPE address AS (
    street VARCHAR(100),
    city VARCHAR(50),
    state CHAR(2),
    zip VARCHAR(10)
);
```

CREATE TRIGGER
```sql
CREATE TRIGGER before_insert_employees
    BEFORE INSERT ON employees
    FOR EACH ROW
    EXECUTE FUNCTION validate_employee();
```

## Demo

![Demo video](./assets/demo.gif)

## Simulation parity

### Unsupported SQL expressions

Everything marked as check [x] is explicitly prohibited by the simulator.

- [x] CREATE VIEW
- [X] CREATE INDEX [ASYNC]
  - [X] ASC
  - [X] DESC
  - [X] if table has data
- [x] TRUNCATE
- [x] ALTER SYSTEM
- [x] CREATE TABLE
  - [X] COLLATE
  - [x] AS SELECT
  - [x] INHERIT
  - [X] PARTITION
- [x] CREATE FUNCTION
  - [x] LANGUAGE plpgsql (any language besides sql)
- [X] CREATE TEMPORARY TABLE
- [x] CREATE EXTENSION
- [x] CREATE SEQUENCE
- [x] CREATE MATERIALIZED VIEW
- [x] CREATE TRIGGER
- [x] CREATE TABLESPACE
- [x] CREATE TYPE
- [x] CREATE DATABASE

### Unsupported constraints
- [ ] Foreign keys
- [ ] Exclusion constraints

### Unsupported operations
- [x] VACUUM
- [ ] SAVEPOINT

### Unsupported extensions
This is achieved by disabling "CREATE EXTENSION"
- [x] PL/pgsql
- [x] PostGIS
- [x] PGVector
- [x] PGAudit
- [x] Postgres_FDW
- [x] PGCron
- [x] pg_stat_statements

### Limitations
- [X] CREATE DATABASE: Aurora DSQL supports a single database postgres which is UTF-8 and collation = C only. You can't modify the system timezone and it's set to UTC
- [ ] SET TRANSACTION [ISOLATION LEVEL]: Aurora DSQL isolation level is equivalent to PostgreSQL Repeatable Read. You can't change this isolation level.
- [ ] A transaction can't contain mixed DDL and DML operations
- [ ] A transaction can contain at most 1 DDL statement
- [ ] A transaction cannot modify more than 10,000 rows, and this limit is modified by secondary index entries. For example, consider a table with five columns, where the primary key is the first column, and the fifth column has a secondary index. Given an UPDATE that will change a single row targeting all five columns, the number of rows modified would be two. One for the Primary Key and one for the row in secondary index object. If this same UPDATE affected only the columns without a secondary index, the number of rows modified would be one. This limit applies to all DML statements (INSERT, UPDATE, DELETE).
- [ ] A connection cannot exceed 1 hour.
- [x] AutoVacuuming to keep statistics up to date. Vacuum is not required in Aurora DSQL.