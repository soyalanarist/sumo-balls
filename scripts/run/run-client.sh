#!/bin/bash
# Quick start script for Sumo Balls Client (Online Mode)

echo "Starting Sumo Balls Client (Online Mode)..."
echo ""

# Change to project root
cd "$(dirname "$0")/.." || exit 1

if [ ! -f "./build/sumo_balls" ]; then
  echo "Client binary not found; building Debug config..."
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build -- -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)"
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
