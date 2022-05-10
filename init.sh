#!/bin/bash

# Build mini-aims executable
g++ **/*.cpp -l sqlite3 -o mini-aims

# create database if it doesn't exist
if [ ! -e db.sqlite ]; then
    cat schema.sql | sqlite3 db.sqlite
fi
