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

## Simulation parity: Unsupported SQL expressions

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
