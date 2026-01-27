#!/bin/bash
set -euo pipefail

# Matchmaking gameplay test: launch two clients with distinct configs and
# run indefinitely until "gameWin" (winner banner) appears in either log.
# Optional MAX_DURATION provides a safety timeout (seconds, 0 to disable).
# Prereqs: coordinator + game server running and registered.

# Change to project root
cd "$(dirname "$0")/.." || exit 1

MAX_DURATION=${MAX_DURATION:-0}
CLIENT1_CONFIG=${CLIENT1_CONFIG:-config/config_client1.json}
CLIENT2_CONFIG=${CLIENT2_CONFIG:-config/config_client2.json}
BASE_CONFIG=${BASE_CONFIG:-config.json}

# Seed configs if missing
if [ ! -f "$CLIENT1_CONFIG" ] && [ -f "$BASE_CONFIG" ]; then
  cp "$BASE_CONFIG" "$CLIENT1_CONFIG"
fi
if [ ! -f "$CLIENT2_CONFIG" ] && [ -f "$BASE_CONFIG" ]; then
  cp "$BASE_CONFIG" "$CLIENT2_CONFIG"
fi

cd /home/soyal/sumo-balls

# Ensure coordinator is running
if ! pgrep -f "coordinator-bin" > /dev/null; then
  echo "Starting coordinator..."
  (cd coordinator && ./coordinator-bin > /tmp/coordinator.log 2>&1 &)
  sleep 2
fi

# Ensure game server is running
if ! pgrep -f "sumo_balls_server" > /dev/null; then
  echo "Starting game server..."
  /home/soyal/sumo-balls/build/sumo_balls_server > /tmp/server.log 2>&1 &
  sleep 2
fi

# Register server with coordinator
echo "Registering server with coordinator..."
curl -X POST http://localhost:8888/server/register \
  -H "Content-Type: application/json" \
  -d '{"server_id":"test-server-1","host":"localhost","port":7777,"max_players":8}' \
  -s > /dev/null || echo "Warning: server registration failed (may already be registered)"

sleep 1

echo "Starting matchmaking gameplay test (waits for gameWin)..."

echo "[Client 1] player1 cfg=$CLIENT1_CONFIG (manual matchmaking, DISPLAY=:0 left side)"
DISPLAY=:0 SUMO_MATCHMAKING=0 SUMO_PLAYER_ID=player1 SUMO_CONFIG="$CLIENT1_CONFIG" ./build/sumo_balls > client1.log 2>&1 &
C1=$!

sleep 1

echo "[Client 2] player2 cfg=$CLIENT2_CONFIG (manual matchmaking, DISPLAY=:0 right side)"
DISPLAY=:0 SUMO_MATCHMAKING=0 SUMO_PLAYER_ID=player2 SUMO_CONFIG="$CLIENT2_CONFIG" ./build/sumo_balls > client2.log 2>&1 &
C2=$!

trap 'kill $C1 $C2 2>/dev/null' EXIT

echo "Waiting for gameWin ("won the game" in logs)..."
start_time=$(date +%s)
while true; do
  if grep -q "won the game" client1.log || grep -q "won the game" client2.log; then
    echo "gameWin detected; clients will show Main Menu button"
    echo "Press Ctrl+C to exit, or wait for clients to return to main menu"
    # Wait indefinitely instead of killing clients
    wait
    break
  fi
  if [ "$MAX_DURATION" != "0" ]; then
    now=$(date +%s)
    elapsed=$((now - start_time))
    if [ "$elapsed" -ge "$MAX_DURATION" ]; then
      echo "Max duration $MAX_DURATION reached; stopping clients"
      break
    fi
  fi
  sleep 1
done

echo "Done. Check client1.log / client2.log and coordinator/coordinator.log for details."
