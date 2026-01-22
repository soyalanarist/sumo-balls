#!/bin/bash
# Test script for coordinator API with auth, friends, and lobbies

API="http://localhost:8888"

echo "=== Testing Coordinator API ==="
echo ""

# Test 1: Register user
echo "[1] Registering user 'alice'..."
ALICE_RESPONSE=$(curl -s -X POST "$API/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123","email":"alice@example.com"}')
echo "$ALICE_RESPONSE" | python3 -m json.tool
ALICE_TOKEN=$(echo "$ALICE_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('token', ''))")
echo ""

# Test 2: Register another user
echo "[2] Registering user 'bob'..."
BOB_RESPONSE=$(curl -s -X POST "$API/auth/register" \
  -H "Content-Type: application/json" \
  -d '{"username":"bob","password":"password123","email":"bob@example.com"}')
echo "$BOB_RESPONSE" | python3 -m json.tool
BOB_TOKEN=$(echo "$BOB_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('token', ''))")
echo ""

# Test 3: Login
echo "[3] Logging in as alice..."
LOGIN_RESPONSE=$(curl -s -X POST "$API/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"alice","password":"password123"}')
echo "$LOGIN_RESPONSE" | python3 -m json.tool
echo ""

# Test 4: Get current user
echo "[4] Getting current user info..."
curl -s -X GET "$API/auth/me" \
  -H "Authorization: Bearer $ALICE_TOKEN" | python3 -m json.tool
echo ""

# Test 5: Send friend request
echo "[5] Alice sending friend request to bob..."
curl -s -X POST "$API/friends/send" \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"friend_username":"bob"}' | python3 -m json.tool
echo ""

# Test 6: Bob sees pending request
echo "[6] Bob checking pending requests..."
curl -s -X GET "$API/friends/pending" \
  -H "Authorization: Bearer $BOB_TOKEN" | python3 -m json.tool
echo ""

# Test 7: Bob accepts friend request (get IDs from responses)
echo "[7] Bob accepting friend request..."
ALICE_ID=$(echo "$ALICE_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('user', {}).get('id', 0))")
curl -s -X POST "$API/friends/accept" \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"friend_id\":$ALICE_ID}" | python3 -m json.tool
echo ""

# Test 8: Alice sees friends list
echo "[8] Alice checking friends list..."
curl -s -X GET "$API/friends/list" \
  -H "Authorization: Bearer $ALICE_TOKEN" | python3 -m json.tool
echo ""

# Test 9: Alice creates a lobby
echo "[9] Alice creating a lobby..."
LOBBY_RESPONSE=$(curl -s -X POST "$API/lobby/create" \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"max_players":6}')
echo "$LOBBY_RESPONSE" | python3 -m json.tool
LOBBY_ID=$(echo "$LOBBY_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('lobby', {}).get('id', ''))")
echo ""

# Test 10: Bob joins the lobby
echo "[10] Bob joining Alice's lobby..."
curl -s -X POST "$API/lobby/join" \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d "{\"lobby_id\":\"$LOBBY_ID\"}" | python3 -m json.tool
echo ""

# Test 11: Get lobby details
echo "[11] Getting lobby details..."
curl -s -X GET "$API/lobby/get?lobby_id=$LOBBY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" | python3 -m json.tool
echo ""

# Test 12: Bob sets ready status
echo "[12] Bob setting ready status..."
curl -s -X POST "$API/lobby/ready?lobby_id=$LOBBY_ID" \
  -H "Authorization: Bearer $BOB_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"ready":true}' | python3 -m json.tool
echo ""

# Test 13: Alice sets ready status
echo "[13] Alice setting ready status..."
curl -s -X POST "$API/lobby/ready?lobby_id=$LOBBY_ID" \
  -H "Authorization: Bearer $ALICE_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"ready":true}' | python3 -m json.tool
echo ""

echo "=== All tests complete! ==="
