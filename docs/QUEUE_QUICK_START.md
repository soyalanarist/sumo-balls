# Friend Queue System - Quick Start Examples

## Complete Workflow Examples

### Example 1: Solo Queue

**Player Alice wants to queue alone**

```bash
# 1. Get token from login
TOKEN=$(curl -X POST http://localhost:8888/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}' | jq -r '.token')

# 2. Start solo queue
QUEUE_RESPONSE=$(curl -X POST http://localhost:8888/queue/start \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"queue_type":"solo"}')

QUEUE_ID=$(echo $QUEUE_RESPONSE | jq -r '.queue.id')
echo "Queue ID: $QUEUE_ID"

# 3. Poll for match (every 2 seconds)
curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
  -H "Authorization: Bearer $TOKEN" | jq .

# 4. When match found, accept it
INVITE_ID="invite_..."  # Received from polling
curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"invitation_id\":\"$INVITE_ID\"}"

# 5. Game starts automatically when all 6 players accept
```

### Example 2: Party Queue

**Alice, Bob, Charlie want to play together**

```bash
# 1. Alice creates party
ALICE_TOKEN=$(...)

PARTY_RESPONSE=$(curl -X POST http://localhost:8888/party/create \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{}')

PARTY_ID=$(echo $PARTY_RESPONSE | jq -r '.party.id')
echo "Party ID: $PARTY_ID"

# 2. Alice invites Bob and Charlie
curl -X POST "http://localhost:8888/party/invite?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"friend_username":"bob"}'

curl -X POST "http://localhost:8888/party/invite?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"friend_username":"charlie"}'

# 3. Bob receives notification and accepts invite
BOB_TOKEN=$(...)

# First get pending invites for Bob
INVITES=$(curl -X GET "http://localhost:8888/friends/pending" \
  -H "Authorization: Bearer $BOB_TOKEN" | jq '.requests')

# Find the party invite and accept it
curl -X POST http://localhost:8888/party/accept-invite \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invite_id":"pinv_1698765432_456"}'

# 4. Same for Charlie
CHARLIE_TOKEN=$(...)
curl -X POST http://localhost:8888/party/accept-invite \
  -H "Authorization: Bearer $CHARLIE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invite_id":"pinv_1698765432_789"}'

# 5. Verify party has 3 members
curl -X GET "http://localhost:8888/party/get?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq '.party.members'

# 6. Alice starts queue
QUEUE_RESPONSE=$(curl -X POST http://localhost:8888/queue/start \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"queue_type\":\"party\",\"party_id\":\"$PARTY_ID\"}")

QUEUE_ID=$(echo $QUEUE_RESPONSE | jq -r '.queue.id')
EST_WAIT=$(echo $QUEUE_RESPONSE | jq -r '.estimated_wait_ms')
echo "Queue ID: $QUEUE_ID, ETA: ${EST_WAIT}ms"

# 7. All players poll for match
for PLAYER in "alice" "bob" "charlie"; do
  TOKEN=$(get_token_for $PLAYER)
  echo "Checking $PLAYER status..."
  curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
    -H "Authorization: Bearer $TOKEN"
done

# 8. Match found! All 6 players get invitations
# Each player accepts (or declines)
curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invitation_id":"invite_1698765432_001"}'

curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invitation_id":"invite_1698765432_002"}'

curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $CHARLIE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invitation_id":"invite_1698765432_003"}'

# Similar for the other 3 players...

# 9. When all 6 accept, game starts!
# Clients receive: {"server_host":"127.0.0.1","server_port":7777,"token":"..."}
```

### Example 3: Party Management

**Managing party members**

```bash
ALICE_TOKEN=$(...)
PARTY_ID="party_1698765432_1"

# Get current party info
curl -X GET "http://localhost:8888/party/get?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq '.party'
# Returns: {
#   "id": "party_1698765432_1",
#   "leader_id": 123,
#   "state": "forming",
#   "members": [
#     {"user_id":123,"username":"alice","is_leader":true},
#     {"user_id":456,"username":"bob","is_leader":false},
#     {"user_id":789,"username":"charlie","is_leader":false}
#   ]
# }

# Leader leaves - leadership transfers to oldest member (Bob)
curl -X POST "http://localhost:8888/party/leave?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN"

# Verify Bob is now leader
curl -X GET "http://localhost:8888/party/get?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $(get_token_for bob)" | jq '.party.leader_id'
# Returns: 456

# Member leaves (Charlie)
CHARLIE_TOKEN=$(...)
curl -X POST "http://localhost:8888/party/leave?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $CHARLIE_TOKEN"

# Check if party still exists with Bob
curl -X GET "http://localhost:8888/party/get?party_id=$PARTY_ID" \
  -H "Authorization: Bearer $(get_token_for bob)" | jq '.party.members | length'
# Returns: 1 (Bob is alone now)
```

### Example 4: Handling Declines

**Player declines a match**

```bash
# During 30-second match acceptance window:
ALICE_TOKEN=$(...)
INVITE_ID="invite_1698765432_001"

# Alice accepts
curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"invitation_id\":\"$INVITE_ID\"}"

# Bob declines
BOB_TOKEN=$(...)
curl -X POST http://localhost:8888/queue/decline-match \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"invitation_id\":\"invite_1698765432_002\"}"

# Result:
# - Entire match is cancelled
# - All 6 players return to queued state
# - They re-queue automatically (for solo) or wait for leader (for party)
# - After cooldown, next matchmaking cycle finds new match
```

### Example 5: Checking Queue Status

**Polling for real-time updates**

```bash
ALICE_TOKEN=$(...)
QUEUE_ID="queue_1698765432_1"

# Initial check
curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq .
# Returns: {
#   "success": true,
#   "queue": {
#     "id": "queue_1698765432_1",
#     "state": "queued",
#     "queue_type": "party",
#     "estimated_wait_ms": 30000
#   }
# }

# After 30 seconds, check again
sleep 30
curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq .
# Returns: {
#   "success": true,
#   "queue": {
#     "state": "match_found",  ← Match found!
#     "estimated_wait_ms": 0
#   }
#   "invitation": {  ← Match invitation included
#     "id": "invite_1698765432_001",
#     "lobby_id": "lobby_match_1698765432_1",
#     "seconds_left": 28  ← 30 seconds to decide
#   }
# }

# Check after accepting
curl -X POST http://localhost:8888/queue/accept-match \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"invitation_id":"invite_1698765432_001"}'

sleep 2
curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq .
# Returns: {
#   "success": true,
#   "queue": {
#     "state": "accepted",  ← Waiting for all players
#     "estimated_wait_ms": 0
#   }
# }

# When all 6 accept
curl -X GET "http://localhost:8888/queue/status?queue_id=$QUEUE_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | jq .
# Returns: {
#   "success": true,
#   "queue": {
#     "state": "ready",  ← All players accepted!
#     "estimated_wait_ms": 0
#   },
#   "game_server": {
#     "host": "127.0.0.1",
#     "port": 7777,
#     "token": "game_token_xyz"
#   }
# }
```

## Testing Scenarios

### Scenario 1: 6 Solo Players
```bash
for i in {1..6}; do
  TOKEN=$(curl -X POST http://localhost:8888/auth/login \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"player$i\",\"password\":\"pass\"}" | jq -r '.token')
  
  curl -X POST http://localhost:8888/queue/start \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d '{"queue_type":"solo"}'
done

# Wait 30-45 seconds
# All 6 should get match invitation
```

### Scenario 2: 2 Parties of 3
```bash
# Party 1: Alice, Bob, Charlie
# Party 2: Dave, Eve, Frank
# Both queue
# Should match immediately!
```

### Scenario 3: Party of 4 + Duo
```bash
# Party 1: 4 players
# Party 2: 2 players
# Should match immediately!
```

### Scenario 4: Mixed Acceptance
```bash
# Match found: 6 players
# Players 1-5 accept
# Player 6 declines
# Result: Match cancelled, all 6 re-queue
```

## Debugging

### Check Party State
```bash
curl -X GET "http://localhost:8888/party/get?party_id=party_..." \
  -H "Authorization: Bearer $TOKEN" | jq '.party'
```

### Check Queue State
```bash
curl -X GET "http://localhost:8888/queue/status?queue_id=queue_..." \
  -H "Authorization: Bearer $TOKEN" | jq '.queue'
```

### Check Database Directly
```bash
sqlite3 coordinator.db "SELECT * FROM parties LIMIT 5;"
sqlite3 coordinator.db "SELECT * FROM queues WHERE state='queued';"
sqlite3 coordinator.db "SELECT * FROM match_invitations WHERE state='pending';"
```

### Check Server Logs
```bash
tail -f <path-to-coordinator-log>
# Look for:
# [Party] Party created: party_...
# [Queue] Started queued queue: queue_...
# [Queue] Match found: 6 players
# [Queue] Invitation sent to user ...
```

## Performance Testing

### Load Test: 100 Solo Queues
```bash
for i in {1..100}; do
  TOKEN=$(get_token_for_user_i)
  curl -X POST http://localhost:8888/queue/start \
    -H "Authorization: Bearer $TOKEN" \
    -H "Content-Type: application/json" \
    -d '{"queue_type":"solo"}' &
done
wait

# Coordinator should:
# - Create ~100 queues
# - Find ~16 matches (100/6 = 16 remainder 4)
# - Send ~96 invitations
# - Complete in <5 seconds
```

### Concurrent Party Operations
```bash
# Have 50 users simultaneously:
# 1. Create parties
# 2. Invite friends
# 3. Accept invites
# 4. Queue

# Should handle without race conditions or data corruption
```

## Troubleshooting

### "Only party leader can queue"
**Problem:** Non-leader tried to queue
**Solution:** Have the party leader (creator) call `/queue/start`

### "Party must have 1-6 members"
**Problem:** Party empty or too large
**Solution:** Check party size with `/party/get`

### "Invitation expired"
**Problem:** Didn't accept/decline in 30 seconds
**Solution:** More responsive UI with countdown timer

### No matches after 2+ minutes
**Problem:** Not enough players queuing
**Solution:** 
1. Check coordinator logs for matchmaking errors
2. Verify queue service is running
3. Check database for stale queues

## Next Steps

1. Implement client UI for:
   - Party creation/management screens
   - Queue status display
   - Match acceptance dialog (30-second timer)
   - Integration with game lobby

2. Add WebSocket support for:
   - Real-time match notifications
   - Avoid polling overhead
   - Instant updates for all parties

3. Implement game server integration:
   - Receive lobby creation signals
   - Get player list from coordinator
   - Start game when ready
   - Report game completion
