#!/bin/bash

set -e

echo "\nGet manager info"
curl --location 'localhost:11110/info'

echo "\n\nGet server info"
curl --location 'localhost:11110/server'

echo "\n\nStart a new server instance"
curl --location 'localhost:11110/server' \
--header 'Content-Type: application/json' \
--data '{
    "server-port": 7777
}'

echo "\n\nGet running server info"
curl --location 'localhost:11110/server'

sleep 3
echo "\n\nGet running server logs"
curl --location 'localhost:11110/server/logs'
