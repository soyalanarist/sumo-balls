# Friend Queue System - Implementation Guide

## Overview

The friend queue system enables players to:
1. **Create parties** with their friends (1-6 players)
2. **Queue together** as a party or solo
3. **Auto-match** when 6 players are available
4. **Accept/decline matches** in real-time
5. **Automatically join lobbies** when all players accept

## API Endpoints

### Party Management

#### Create Party
```http
POST /party/create
Authorization: Bearer {token}
Content-Type: application/json

{
  "friend_usernames": ["alice", "bob"]  // Optional: initial invites
}
```

**Response:**
```json
{
  "success": true,
  "party": {
    "id": "party_1698765432_1",
    "leader_id": 123,
    "state": "forming",
    "created_at": "2026-01-25T12:00:00Z",
    "updated_at": "2026-01-25T12:00:00Z",
    "members": [
      {
        "user_id": 123,
        "username": "alice",
        "handle": "Alice#1234",
        "is_leader": true,
        "joined_at": "2026-01-25T12:00:00Z"
      }
    ]
  }
}
```

#### Get Party Info
```http
GET /party/get?party_id=party_1698765432_1
Authorization: Bearer {token}
```

#### Invite Friend to Party
```http
POST /party/invite?party_id=party_1698765432_1
Authorization: Bearer {token}
Content-Type: application/json

{
  "friend_username": "charlie"
}
```

#### Accept Party Invite
```http
POST /party/accept-invite
Authorization: Bearer {token}
Content-Type: application/json

{
  "invite_id": "pinv_1698765432_456"
}
```

#### Leave Party
```http
POST /party/leave?party_id=party_1698765432_1
Authorization: Bearer {token}
```

### Queue Management

#### Start Queue (Solo)
```http
POST /queue/start
Authorization: Bearer {token}
Content-Type: application/json

{
  "queue_type": "solo"
}
```

#### Start Queue (Party)
```http
POST /queue/start
Authorization: Bearer {token}
Content-Type: application/json

{
  "queue_type": "party",
  "party_id": "party_1698765432_1"
}
```

**Response:**
```json
{
  "success": true,
  "queue": {
    "id": "queue_1698765432_1",
    "user_id": 123,
    "party_id": "party_1698765432_1",
    "queue_type": "party",
    "state": "queued",
    "estimated_wait_ms": 30000,
    "created_at": "2026-01-25T12:00:00Z",
    "updated_at": "2026-01-25T12:00:00Z"
  },
  "estimated_wait_ms": 30000
}
```

#### Check Queue Status
```http
GET /queue/status?queue_id=queue_1698765432_1
Authorization: Bearer {token}
```

#### Accept Match Invitation
```http
POST /queue/accept-match
Authorization: Bearer {token}
Content-Type: application/json

{
  "invitation_id": "invite_1698765432_789"
}
```

#### Decline Match Invitation
```http
POST /queue/decline-match
Authorization: Bearer {token}
Content-Type: application/json

{
  "invitation_id": "invite_1698765432_789"
}
```

#### Cancel Queue
```http
POST /queue/cancel?queue_id=queue_1698765432_1
Authorization: Bearer {token}
```

## Client Implementation Flow

### Party Creation Flow
```
User A wants to play with Friends B and C

1. User A: POST /party/create
   → Returns party_id: "party_123_1"

2. User A: POST /party/invite?party_id=party_123_1
   → Invites Friend B

3. User A: POST /party/invite?party_id=party_123_1
   → Invites Friend C

4. Friend B receives notification with invite_id
   → POST /party/accept-invite
   → Joins party

5. Friend C receives notification with invite_id
   → POST /party/accept-invite
   → Joins party

6. All 3 now in party, ready to queue
```

### Queue Flow
```
Party of 3 wants to find a match

1. User A (party leader): POST /queue/start
   {
     "queue_type": "party",
     "party_id": "party_123_1"
   }
   → Returns queue_id: "queue_456_1"
   → ETA: ~45 seconds

2. Polling: GET /queue/status?queue_id=queue_456_1
   → state: "queued"
   → estimated_wait_ms: 25000

3. [After 25-45 seconds]
   Match found with another party of 3!

4. All 6 players receive match invitation
   {
     "id": "invite_789_1",
     "queue_id": "queue_456_1",
     "user_id": 123,
     "lobby_id": "lobby_match_1698765432_1",
     "state": "pending",
     "expires_at": "2026-01-25T12:01:30Z"
   }

5. Each player has 30 seconds to accept/decline
   Player A: POST /queue/accept-match
   Player B: POST /queue/accept-match
   Player C: POST /queue/accept-match
   Player D: POST /queue/accept-match
   Player E: POST /queue/accept-match
   Player F: POST /queue/accept-match

6. All 6 accepted → Lobby state becomes "starting"
   → Game server receives signal to begin
   → Clients receive server address and game token
   → Game starts!
```

## State Transitions

### Queue States
```
queued          ← Initial state, waiting for match
  ↓
match_found     ← Match found, invitation sent (30s window)
  ↓
accepted        ← Player accepted, waiting for all players
  ↓
ready           ← All accepted, entering lobby
  ↓
in_game         ← Game started
  
DECLINED/CANCELLED → Requeue or end
```

### Party States
```
forming         ← Party created, members inviting
  ↓
queued          ← Party queued for match
  ↓
in_match        ← Playing together
  ↓
disbanded       ← Party dissolved (manual or after game)
```

### Match Invitation States
```
pending         ← Sent, waiting for player response (30s)
  ↓
accepted        ← Player accepted
  ↓
declined        ← Player declined, match cancelled
expired         ← Player didn't respond in 30s
```

## Database Schema

### parties
```
id              TEXT PRIMARY KEY
leader_id       INTEGER (user who created party)
state           TEXT ('forming', 'queued', 'in_match', 'disbanded')
created_at      DATETIME
updated_at      DATETIME
```

### party_members
```
id              INTEGER PRIMARY KEY
party_id        TEXT (FK parties)
user_id         INTEGER (FK users)
joined_at       DATETIME
```

### party_invites
```
id              TEXT PRIMARY KEY
party_id        TEXT (FK parties)
from_user_id    INTEGER (FK users)
to_user_id      INTEGER (FK users)
state           TEXT ('pending', 'accepted', 'declined', 'expired')
created_at      DATETIME
expires_at      DATETIME (24 hours from creation)
```

### queues
```
id              TEXT PRIMARY KEY
user_id         INTEGER (FK users - initiator or leader)
party_id        TEXT (FK parties, nullable for solo)
queue_type      TEXT ('solo', 'party')
state           TEXT ('queued', 'match_found', 'accepted', 'declined', 'ready', 'cancelled')
estimated_wait_ms INTEGER
created_at      DATETIME
updated_at      DATETIME
```

### match_invitations
```
id              TEXT PRIMARY KEY
queue_id        TEXT (FK queues)
user_id         INTEGER (FK users)
lobby_id        TEXT (FK lobbies)
state           TEXT ('pending', 'accepted', 'declined', 'expired')
created_at      DATETIME
expires_at      DATETIME (30 seconds from creation)
```

## Matchmaking Algorithm

### Every 2 seconds:
1. **Collect** all queued players/parties
2. **Group** by party (solo players each form their own group)
3. **Find matches** that sum to 6:
   - 6 solo players
   - 3 parties of 2
   - 2 parties of 3
   - 1 party of 4 + 1 party of 2
   - etc.
4. **Create lobby** for matched group
5. **Send invitations** (30-second window)
6. **Collect acceptances**
7. **Start game** when all accept

### Match Quality Score (Future)
- ELO/Rating balancing
- Regional latency preferences
- Skill spread validation

## Error Handling

### Common Errors

#### "Only party leader can queue"
- Only the player who created the party can start the queue
- Solution: Have the leader call `/queue/start`

#### "Party must have 1-6 members"
- Empty party or too many players
- Solution: Add members (1-6) before queuing

#### "Invitation expired"
- 30 seconds elapsed without accepting/declining
- Solution: Re-queue and accept faster next time

#### "Queue not found"
- Queue was cancelled or doesn't exist
- Solution: Check queue_id is correct

### Edge Cases Handled

**Player Disconnects During Queue:**
- Automatic removal on inactivity
- Party continues queuing without them

**Player Declines Match:**
- Entire party's match is cancelled
- Party remains, can re-queue

**Party Leader Leaves:**
- Leadership transfers to oldest member
- If party becomes 1 person, treated as solo queue

**Match Acceptance Timeout:**
- No response in 30 seconds = declined
- Match cancelled, players re-queued (with cooldown)

## Testing Checklist

- [ ] Create party with multiple friends
- [ ] Invite friend and accept
- [ ] Leave party as member
- [ ] Leave party as leader (transfer leadership)
- [ ] Queue solo
- [ ] Queue as party
- [ ] Check queue status
- [ ] Receive match invitation
- [ ] Accept match
- [ ] Decline match
- [ ] Accept match, others decline
- [ ] All players accept and join lobby
- [ ] Game starts with 6 players

## Future Enhancements

1. **Ranked vs Casual Queues**
   - Separate queue modes
   - Different ELO calculations

2. **Voice Chat Integration**
   - WebRTC server via coordinator
   - Auto-connect party members during game

3. **Queue Statistics**
   - Average queue times
   - Acceptance rates
   - Win/loss tracking

4. **Anti-Cheat Integration**
   - Validate players on match creation
   - Game token verification

5. **Replay System**
   - Auto-upload replays to coordinator
   - Share with friends

6. **Custom Lobbies**
   - Invite-specific players
   - Private matches
   - Practice games
