#!/bin/bash

# Register a mock game server with the coordinator

SERVER_ID="server_test_1"
HOST="127.0.0.1"
PORT=9999

echo "Registering server $SERVER_ID at $HOST:$PORT..."

curl -X POST http://localhost:8888/server/register \
  -H "Content-Type: application/json" \
  -d "{\"server_id\":\"$SERVER_ID\",\"host\":\"$HOST\",\"port\":$PORT,\"max_players\":4}"

echo ""
echo "Server registered! Check status at: http://localhost:8888/matches"
