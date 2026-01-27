#!/bin/bash
# Quick-start script for Sumo Balls with Google OAuth

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

echo "=========================================="
echo "Sumo Balls - Google OAuth Quick Start"
echo "=========================================="
echo ""

# Check if credentials are set
if [ -z "$GOOGLE_CLIENT_ID" ] || [ -z "$GOOGLE_CLIENT_SECRET" ]; then
    echo "⚠ Google OAuth credentials not configured"
    echo ""
    echo "To set up Google OAuth:"
    echo "1. Run: bash scripts/setup-oauth.sh"
    echo "2. Follow the prompts to enter your credentials"
    echo "3. Then run this script again"
    echo ""
    echo "For detailed instructions, see:"
    echo "  docs/GOOGLE_OAUTH_SETUP.md"
    echo ""
    read -p "Continue without OAuth? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 0
    fi
else
    echo "✓ Google OAuth configured"
    echo "  Client ID: ${GOOGLE_CLIENT_ID:0:20}..."
    echo ""
fi

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

# Build the project
echo "Building Sumo Balls..."
cd build
cmake -DCMAKE_BUILD_TYPE=Debug .. > /dev/null 2>&1 || (cmake -DCMAKE_BUILD_TYPE=Debug .. && echo "CMake configuration failed")
make -j4 || (echo "Build failed" && exit 1)
echo "✓ Build successful"
echo ""

# Check if coordinator is running
COORDINATOR_PORT=8888
GAMESERVER_PORT=9999

check_port() {
    nc -z localhost "$1" 2>/dev/null
}

if check_port $COORDINATOR_PORT; then
    echo "✓ Coordinator is already running on :$COORDINATOR_PORT"
else
    echo "Starting Coordinator..."
    cd ../coordinator
    if [ -f ".env" ]; then
        source .env
    fi
    ./run.sh > /tmp/sumo-coordinator.log 2>&1 &
    COORDINATOR_PID=$!
    echo "  Started with PID $COORDINATOR_PID"
    
    # Wait for coordinator to start
    sleep 2
    if check_port $COORDINATOR_PORT; then
        echo "✓ Coordinator ready on :$COORDINATOR_PORT"
    else
        echo "✗ Coordinator failed to start. Check /tmp/sumo-coordinator.log"
        exit 1
    fi
    cd ../build
fi

if check_port $GAMESERVER_PORT; then
    echo "✓ Game Server is already running on :$GAMESERVER_PORT"
else
    echo "Starting Game Server..."
    ./sumo_balls_server $GAMESERVER_PORT > /tmp/sumo-server.log 2>&1 &
    SERVER_PID=$!
    echo "  Started with PID $SERVER_PID"
    
    # Wait for server to start
    sleep 1
    if check_port $GAMESERVER_PORT; then
        echo "✓ Game Server ready on :$GAMESERVER_PORT"
    else
        echo "✗ Game Server failed to start. Check /tmp/sumo-server.log"
        exit 1
    fi
fi

echo ""
echo "=========================================="
echo "All systems ready!"
echo "=========================================="
echo ""
echo "Starting game client..."
echo ""
echo "Features available:"
if [ -n "$GOOGLE_CLIENT_ID" ] && [ -n "$GOOGLE_CLIENT_SECRET" ]; then
    echo "  ✓ Google OAuth sign-in"
else
    echo "  ○ Google OAuth sign-in (not configured)"
fi
echo "  ✓ Username/password login"
echo "  ✓ Offline play"
echo "  ✓ Multiplayer (server running on :$GAMESERVER_PORT)"
echo ""
echo "Launch options:"
echo "  1. Click 'Play' for offline mode"
echo "  2. Click 'Sign in with Google' to use OAuth (opens in Windows browser via WSL)"
echo "  3. Enter username/password for traditional login"
echo ""

./sumo_balls
