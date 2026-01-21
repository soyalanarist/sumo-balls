#!/bin/bash

# Kill any existing processes
pkill -f sumo_balls_server || true
pkill -f "sumo_balls$" || true
sleep 1

echo "Starting server..."
./build/sumo_balls_server 7777 &
SERVER_PID=$!
sleep 2

echo "Starting client..."
SUMO_ONLINE=1 SUMO_HOST=127.0.0.1 SUMO_PORT=7777 timeout 10 ./build/sumo_balls 2>&1 | head -100 &
CLIENT_PID=$!

sleep 5

echo "Killing processes..."
kill $SERVER_PID $CLIENT_PID 2>/dev/null || true
wait 2>/dev/null || true

echo "Done"
