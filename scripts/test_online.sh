#!/bin/bash
# Test script for online multiplayer functionality

# Change to project root
cd "$(dirname "$0")/.." || exit 1

# Kill any existing processes
pkill -f sumo_balls_server || true
pkill -f "sumo_balls$" || true
sleep 1

echo "Starting server..."
./build/sumo_balls_server 7777 &
SERVER_PID=$!
sleep 2

echo "Starting client..."
./build/sumo_balls &
CLIENT_PID=$!

# Wait a bit for connection
sleep 5

# Cleanup
echo "Cleaning up..."
kill $SERVER_PID $CLIENT_PID 2>/dev/null || true

echo "Test complete"
