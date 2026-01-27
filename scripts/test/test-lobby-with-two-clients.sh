#!/bin/bash

# Test lobby system with two clients (Alice and Bob)
# Usage: ./scripts/test-lobby-with-two-clients.sh

cd "$(dirname "$0")/.."

echo "=== Testing Lobby System with Alice and Bob ==="
echo ""
echo "This script will launch two game clients:"
echo "  - Client 1 (Alice): config/config_client1.json"
echo "  - Client 2 (Bob):   config/config_client2.json"
echo ""
echo "Credentials:"
echo "  Alice: alice / alice123"
echo "  Bob:   bob / bob123"
echo ""
echo "Test Steps:"
echo "  1. Both clients will auto-login"
echo "  2. In Alice's client: Navigate to online lobby, create lobby"
echo "  3. In Bob's client: Navigate to online lobby, join Alice's lobby"
echo "  4. Both clients: Click ready, then start game"
echo ""
echo "Press Enter to launch Client 1 (Alice)..."
read

# Launch client 1 (Alice)
echo "Launching Client 1 (Alice)..."
CONFIG_PATH=config/config_client1.json DISABLE_GOOGLE_AUTH=1 ./build/sumo_balls &
CLIENT1_PID=$!
echo "Client 1 PID: $CLIENT1_PID"

sleep 2

echo ""
echo "Press Enter to launch Client 2 (Bob)..."
read

# Launch client 2 (Bob)
echo "Launching Client 2 (Bob)..."
CONFIG_PATH=config/config_client2.json DISABLE_GOOGLE_AUTH=1 ./build/sumo_balls &
CLIENT2_PID=$!
echo "Client 2 PID: $CLIENT2_PID"

echo ""
echo "=== Both clients launched ==="
echo "Client 1 (Alice) PID: $CLIENT1_PID"
echo "Client 2 (Bob) PID:   $CLIENT2_PID"
echo ""
echo "Press Ctrl+C to stop watching (clients will continue running)"
echo "To kill both clients: kill $CLIENT1_PID $CLIENT2_PID"
echo ""

# Wait for user to stop watching
wait
