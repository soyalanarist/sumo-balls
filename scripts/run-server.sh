#!/bin/bash
# Quick start script for Sumo Balls Server

PORT="${1:-7777}"

echo "Starting Sumo Balls Server on port $PORT..."
echo "Press Ctrl+C to stop"
echo ""

# Change to project root
cd "$(dirname "$0")/.." || exit 1

if [ ! -f "./build/sumo_balls_server" ]; then
    echo "Error: Server executable not found!"
    echo "Please build the game first:"
    echo "  mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

./build/sumo_balls_server "$PORT"
