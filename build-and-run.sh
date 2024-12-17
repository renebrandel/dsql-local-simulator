#!/bin/sh

docker kill my_postgres
docker rm my_postgres
docker build -t postgres-with-extension .
docker run -d -p 5432:5432 --name my_postgres postgres-with-extension
docker ps
sleep 1
psql postgres://postgres:postgres@localhost:5432/mydb -c "DROP EXTENSION IF EXISTS block_unsupported; CREATE EXTENSION block_unsupported;"
psql postgres://postgres:postgres@localhost:5432/mydb