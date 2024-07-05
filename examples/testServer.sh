#!/bin/bash

echo "\nGet server info"
curl --location 'localhost:11110/server'

echo "\n\nStart a new server instance"
curl --location 'localhost:11110/server' \
--header 'Content-Type: application/json' \
--data '{
    "server-port": 7777
}'

echo "\n\nGet running server info"
curl --location 'localhost:11110/server'

