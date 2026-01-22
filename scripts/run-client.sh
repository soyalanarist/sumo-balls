#!/bin/bash
# Quick start script for Sumo Balls Client (Online Mode)

echo "Starting Sumo Balls Client (Online Mode)..."
echo ""

# Change to project root
cd "$(dirname "$0")/.." || exit 1

if [ ! -f "./build/sumo_balls" ]; then
    echo "Error: Client executable not found!"
    echo "Please build the game first:"
    echo "  mkdir -p build && cd build && cmake .. && make"
    exit 1
fi

if [ ! -f "./config.json" ]; then
    echo "Warning: config.json not found. Creating default..."
    cat > config.json << 'EOF'
{
  "leftyMode": false,
  "fullscreen": false,
  "playerColorIndex": 5,
  "onlineEnabled": true,
  "onlineHost": "127.0.0.1",
  "onlinePort": 7777
}
EOF
fi

echo "Configuration:"
echo "  Host: $(grep onlineHost config.json | cut -d'"' -f4)"
echo "  Port: $(grep onlinePort config.json | cut -d':' -f2 | tr -d ' ,')"
echo ""
echo "Controls: WASD or Arrow Keys"
echo ""

SUMO_ONLINE=1 ./build/sumo_balls
