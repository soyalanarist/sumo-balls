#!/bin/bash
# Launch two game clients with matchmaking enabled

# Change to project root
cd "$(dirname "$0")/.." || exit 1

# Allow per-client configs so inputs/options don't conflict
CLIENT1_CONFIG=${CLIENT1_CONFIG:-config/config_client1.json}
CLIENT2_CONFIG=${CLIENT2_CONFIG:-config/config_client2.json}
BASE_CONFIG=${BASE_CONFIG:-config.json}

# If custom configs don't exist yet, seed them from the base config
if [ ! -f "$CLIENT1_CONFIG" ] && [ -f "$BASE_CONFIG" ]; then
	cp "$BASE_CONFIG" "$CLIENT1_CONFIG"
fi
if [ ! -f "$CLIENT2_CONFIG" ] && [ -f "$BASE_CONFIG" ]; then
	cp "$BASE_CONFIG" "$CLIENT2_CONFIG"
fi

echo "Starting matchmaking clients..."
echo ""

# Launch client 1
echo "[Client 1] Starting with player ID: player1 (config: $CLIENT1_CONFIG)"
cd /home/soyal/sumo-balls
DISPLAY=:0 SUMO_MATCHMAKING=1 SUMO_PLAYER_ID=player1 SUMO_CONFIG="$CLIENT1_CONFIG" ./build/sumo_balls &
CLIENT1_PID=$!

sleep 1

# Launch client 2
echo "[Client 2] Starting with player ID: player2 (config: $CLIENT2_CONFIG)"
DISPLAY=:0 SUMO_MATCHMAKING=1 SUMO_PLAYER_ID=player2 SUMO_CONFIG="$CLIENT2_CONFIG" ./build/sumo_balls &
CLIENT2_PID=$!

echo ""
echo "Clients launched!"
echo "  Client 1 PID: $CLIENT1_PID"
echo "  Client 2 PID: $CLIENT2_PID"
echo ""
echo "Both clients should enqueue and match within 2-3 seconds."
echo "Watch the coordinator log: tail -f coordinator/coordinator.log"
echo ""
echo "Press Enter to stop clients..."
read

kill $CLIENT1_PID $CLIENT2_PID 2>/dev/null
echo "Clients stopped."
