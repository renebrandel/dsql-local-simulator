# DSQL Local Simulator (**un**official)

A PostgreSQL Docker container hosted on port 5432 with an extension to block unsupported SQL statements in Amazon Aurora DSQL.

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
3. Try an unsupported statement and see it fail!

## Simulation parity: Unsupported SQL expressions

Everything marked as check [x] is explicitly prohibited by the simulator.

- [x] CREATE VIEW
- [ ] CREATE INDEX [ASYNC]
  - [ ] ASC
  - [ ] DESC
  - [ ] if table has data
- [x] TRUNCATE
- [x] ALTER SYSTEM
  - [ ] All alter system is blocked
- [ ] CREATE TABLE
  - [ ] COLLATE
  - [ ] AS SELECT
  - [ ] INHERIT
  - [ ] PARTITION
- [ ] CREATE FUNCTION
  - [ ] LANGUAGE plpgsql (any language besides sql)
- [x] CREATE TEMPORARY TABLE
- [x] CREATE EXTENSION
- [x] CREATE SEQUENCE
- [x] CREATE MATERIALIZED VIEW
- [x] CREATE TRIGGER
- [x] CREATE TABLESPACE
- [x] CREATE TYPE
- [x] CREATE DATABASE# dsql-local-simulator
