# Friend Queue System Design

## Overview
A queuing system that allows players to queue for matches individually or with their friends as a party, then automatically places them into game lobbies when enough players are ready.

## Architecture

### 1. Queue Types

**Solo Queue:**
- Individual player queues alone
- Matched with other players based on rank/rating (for future)

**Party Queue:**
- Group of 1-6 friends queue together
- Party members must confirm readiness together
- Party stays together through entire queue and match
- One player (party leader) initiates queue
- Party must have 2-6 members (can expand when needed)

### 2. Queue States

```
QUEUED        → Player/party waiting for match
MATCH_FOUND   → Match found, waiting for players to accept
ACCEPTED      → Player accepted, waiting for all to accept
DECLINED      → Player declined match (rejoins queue after delay)
READY         → All players accepted, entering lobby
IN_GAME       → Game started, no longer in queue system
CANCELLED     → Queue cancelled, player left queue
```

### 3. Database Schema Additions

```sql
-- Queue state tracking
CREATE TABLE IF NOT EXISTS queues (
    id TEXT PRIMARY KEY,                    -- "queue_TIMESTAMP_USERID"
    user_id INTEGER NOT NULL,               -- Solo queue: initiator, Party queue: leader
    party_id TEXT,                          -- NULL for solo, "party_TIMESTAMP_USERID" for parties
    queue_type TEXT NOT NULL,               -- 'solo', 'party'
    state TEXT NOT NULL,                    -- 'queued', 'match_found', 'accepted', 'declined', 'ready', 'cancelled'
    estimated_wait_ms INTEGER DEFAULT 0,    -- Estimated time to match (ms)
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Party membership
CREATE TABLE IF NOT EXISTS parties (
    id TEXT PRIMARY KEY,                    -- "party_TIMESTAMP_USERID"
    leader_id INTEGER NOT NULL,             -- Party leader (can change)
    state TEXT NOT NULL,                    -- 'forming', 'queued', 'in_match', 'disbanded'
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (leader_id) REFERENCES users(id)
);

CREATE TABLE IF NOT EXISTS party_members (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    party_id TEXT NOT NULL,
    user_id INTEGER NOT NULL,
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (party_id) REFERENCES parties(id),
    FOREIGN KEY (user_id) REFERENCES users(id),
    UNIQUE(party_id, user_id)
);

-- Match invitations (10-30 second acceptance window)
CREATE TABLE IF NOT EXISTS match_invitations (
    id TEXT PRIMARY KEY,                    -- "match_TIMESTAMP_RANDOMID"
    queue_id TEXT NOT NULL,
    user_id INTEGER NOT NULL,               -- For party: inserted for each member
    lobby_id TEXT,                          -- Target lobby to join (created when match found)
    state TEXT NOT NULL,                    -- 'pending', 'accepted', 'declined', 'expired'
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    expires_at DATETIME NOT NULL,           -- 10-30 seconds from creation
    FOREIGN KEY (queue_id) REFERENCES queues(id),
    FOREIGN KEY (user_id) REFERENCES users(id)
);
```

### 4. API Endpoints

#### Party Management
- **POST /api/party/create** - Create party (solo player initiates)
- **POST /api/party/invite** - Invite friend to party
- **POST /api/party/accept-invite** - Friend accepts party invite
- **POST /api/party/leave** - Leave party
- **POST /api/party/kick** - Leader kicks member
- **GET /api/party/info** - Get party details
- **POST /api/party/transfer-leader** - Transfer leadership

#### Queue Management
- **POST /api/queue/start** - Start queuing (solo or with party)
- **POST /api/queue/cancel** - Cancel queue
- **POST /api/queue/status** - Get queue status & ETA
- **POST /api/queue/accept-match** - Accept match invitation
- **POST /api/queue/decline-match** - Decline match
- **GET /api/queues/active** - List active players/parties queuing

## Queue Matching Algorithm

### Matching Strategy
1. **Collect candidates** - Gather all players/parties with same queue size
2. **Group by size** - Create buckets: solo (1), duos (2), trios (3), quads (4), etc.
3. **Fill lobbies** - Match groups that sum to 6 players
   - 6 solo players
   - 3 duos
   - 2 trios
   - 1 quad + 1 duo
   - 1 quad + 2 solos
   - 1 trio + 1 duo + 1 solo
   - etc.
4. **Fallback** - If exact match not found within timeout, create mixed groups
5. **Send invitations** - Send 10-30 second match invitations to all players
6. **Collect acceptances** - Wait for all players to accept
7. **Create lobby** - Auto-create lobby and add all accepted players
8. **Start game** - Notify game server when 6 players confirmed ready

### Match Timeout Logic
```
t=0s      Player/party enters queue
t=30s     First match attempt (exact group matching)
t=60s     Second attempt (allow similar sizes)
t=120s    Match with any available players (timeout fallback)
t=300s    Queue cancelled with notification (optional)
```

## Client State Management

### UI States
1. **Lobby Screen** - Not queuing
2. **Party Formation** - Creating/managing party
3. **Queue Waiting** - "In queue... ETA: 1m 20s"
4. **Match Found** - "Match found! Accept?" (10-30 sec countdown)
5. **Game Starting** - Transition to game lobby
6. **In Game** - Playing match

### Party Invite UI
```
From: Alice
"Wants to party up!"
[Accept] [Decline]
```

### Queue Status UI
```
Queue Type: Party (3 players)
Est. Wait: 45 seconds
Members:
  You (Leader)
  Bob
  Charlie

[Cancel Queue]
```

### Match Accept UI
```
⭐ MATCH FOUND! 
30 seconds to accept

Loading servers...

[ACCEPT] [DECLINE]
```

## Game Server Integration

### Lobby Auto-Creation
- Queue system creates lobby when match found
- Passes server address to client
- Client joins lobby via game server (ENet)
- Game server doesn't know about queue system

### State Transitions
```
Queue System                    Game Server
─────────────────────────────────────────
Match Found
  ↓
Create Lobby ─────────→ Game Server
                       (Create game room)
Players Accept
  ↓
Send Server Addr ─────→ Clients
                       (ENet UDP connect)
  ↓
All Connected
  ↓
Start Game ───────────→ Server (Begin tick)
```

## Implementation Phases

### Phase 1: Party System (Week 1)
- [x] Party creation/management DB
- [x] Party invite/accept API
- [x] Party state management
- [x] Party member tracking

### Phase 2: Queue System (Week 2)
- [x] Queue DB schema
- [x] Queue start/cancel API
- [x] Queue status tracking
- [x] Match invitation system
- [x] Acceptance/decline handling

### Phase 3: Matchmaking Algorithm (Week 3)
- [x] Queue polling service
- [x] Group matching logic
- [x] Lobby auto-creation
- [x] Timeout handling

### Phase 4: Client Integration (Week 4)
- [x] Party UI components
- [x] Queue status display
- [x] Match accept/decline buttons
- [x] Game start transition

## Handling Edge Cases

### Dropped Players
```
During Queue:
  Player disconnects → Auto-cancelled from queue
  
During Match Accept:
  Player declines → Remaining players re-queued
  Player timeout (no response in 30s) → Treated as decline
  
During Game:
  Player crashes → Game continues (empty slot)
```

### Party Dissolution
```
Party Leader Leaves:
  → Leadership transfers to oldest member
  → Party continues if ≥2 members
  → If 1 member left, party becomes solo queue
  
Other Member Leaves:
  → Removed from party
  → If in queue, player re-queued as solo
  → Party continues with remaining members
```

### Stale Data Cleanup
```
Every 5 minutes:
  - Delete expired match invitations (>30s old)
  - Delete cancelled queues (>10min old)
  - Mark inactive parties as disbanded (>30min idle)
```

## Future Enhancements

1. **Rating-Based Matching** - ELO/MMR balancing
2. **Regional Queues** - Match players by latency region
3. **Ranked vs Casual** - Separate queue modes
4. **Party Skill Spread** - Prevent smurfing abuse
5. **Queue Statistics** - Track avg queue times, acceptance rates
6. **Replay Upload** - Auto-upload replays to coordinator
7. **Voice Chat Integration** - WebRTC via coordinator
8. **Custom Game Lobbies** - Invite-only, non-ranked matches
