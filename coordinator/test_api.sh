#!/bin/bash

# Test the coordinator API

echo "Testing Coordinator API..."
echo ""

echo "1. Enqueuing player1..."
curl -s -X POST http://localhost:8888/enqueue \
  -H "Content-Type: application/json" \
  -d '{"player_id":"player1","mode":"matchmaking","region":"us-west"}' | python3 -m json.tool

echo ""
echo "2. Enqueuing player2..."
curl -s -X POST http://localhost:8888/enqueue \
  -H "Content-Type: application/json" \
  -d '{"player_id":"player2","mode":"matchmaking","region":"us-west"}' | python3 -m json.tool

echo ""
echo "3. Checking queue status for player1..."
curl -s -X GET "http://localhost:8888/queue/status?player_id=player1" | python3 -m json.tool

echo ""
echo "4. Registering game server..."
curl -s -X POST http://localhost:8888/server/register \
  -H "Content-Type: application/json" \
  -d '{"server_id":"server_west_1","host":"192.168.1.100","port":9999,"max_players":4}' | python3 -m json.tool

echo ""
echo "5. Viewing active matches..."
curl -s -X GET http://localhost:8888/matches | python3 -m json.tool

echo ""
echo "Done!"
