# Use the official PostgreSQL image as the base image
FROM postgres:latest

# Set environment variables
ENV POSTGRES_USER=postgres
ENV POSTGRES_PASSWORD=postgres
ENV POSTGRES_DB=mydb

# Install necessary build tools and PostgreSQL server development libraries
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    postgresql-server-dev-all && \
    rm -rf /var/lib/apt/lists/*

# Copy the extension source code to the container
COPY src/extension /usr/src/extension

# Build and install the PostgreSQL extension
WORKDIR /usr/src/extension
RUN make && make install

# Copy initialization SQL script to Docker entrypoint initdb.d
COPY init_extension.sql /docker-entrypoint-initdb.d/

# Expose PostgreSQL port
EXPOSE 5432
