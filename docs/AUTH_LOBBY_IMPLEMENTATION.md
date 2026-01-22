## Coordinator Auth, Friends & Lobby System - Implementation Summary

### What Was Added

Expanded the coordinator from basic matchmaking to a full social gaming backend with:

1. **Authentication System** (`auth.go`)
   - User registration with password hashing (bcrypt)
   - Login with session tokens
   - Token-based auth middleware
   - 7-day session expiry

2. **Friends System** (`friends.go`)
   - Send friend requests by username
   - Accept/reject friend requests
   - Remove friends
   - View friends list & pending requests

3. **Lobby System** (`lobby.go`)
   - Create lobbies (2-6 players)
   - Join friend lobbies
   - Leave lobbies (auto-delete if host leaves)
   - Ready status per player
   - Real-time lobby state

4. **Database Layer** (`database.go`)
   - SQLite for persistence
   - Tables: users, sessions, friends, lobbies, lobby_members
   - Full CRUD operations

### API Endpoints

#### Authentication (Public)
- `POST /auth/register` - Create account
- `POST /auth/login` - Login and get token
- `POST /auth/logout` - Invalidate session
- `GET /auth/me` - Get current user info

#### Friends (Authenticated)
- `POST /friends/send` - Send friend request
- `POST /friends/accept` - Accept friend request
- `DELETE /friends/remove` - Remove friend
- `GET /friends/list` - Get friends list
- `GET /friends/pending` - Get pending requests

#### Lobbies (Authenticated)
- `POST /lobby/create` - Create new lobby
- `POST /lobby/join` - Join lobby by ID
- `POST /lobby/leave` - Leave lobby
- `POST /lobby/ready?lobby_id=X` - Set ready status
- `GET /lobby/get?lobby_id=X` - Get lobby details

#### Legacy Matchmaking (Backward Compatible)
- `POST /enqueue` - Queue for matchmaking
- `POST /cancel` - Cancel queue
- `GET /queue/status` - Check queue status
- `POST /server/register` - Register game server
- `POST /server/heartbeat` - Server heartbeat
- `GET /matches` - List active matches

### How Authentication Works

**Registration/Login Flow:**
```
Client → POST /auth/register {username, password, email}
       ← {success: true, token: "abc123...", user: {...}}

Client stores token locally
```

**Authenticated Requests:**
```
Client → GET /friends/list
         Header: Authorization: Bearer abc123...
       ← {success: true, friends: [...]}
```

### Database Schema

```sql
users:
  - id (PK)
  - username (unique)
  - password_hash
  - email
  - created_at

sessions:
  - id (PK)
  - user_id (FK)
  - token (unique)
  - expires_at

friends:
  - id (PK)
  - user_id (FK)
  - friend_id (FK)
  - status ('pending'|'accepted')

lobbies:
  - id (PK, text)
  - host_id (FK)
  - state ('waiting'|'starting'|'playing')
  - max_players (default 6)

lobby_members:
  - id (PK)
  - lobby_id (FK)
  - user_id (FK)
  - ready_status (0=not ready, 1=ready)
```

### Testing

**Start coordinator:**
```bash
./coordinator/run.sh
```

**Test new features:**
```bash
./coordinator/test_new_api.sh
```

This creates 2 users, makes them friends, creates a lobby, and has them join + ready up.

### Next Steps for Client Integration

**Client needs to add:**

1. **Login Screen** (before main menu)
   - Username/password input
   - Register button
   - Store token in Settings

2. **Friends UI** (main menu tab)
   - List friends (online status would need websockets)
   - Add friend by username
   - Accept/reject requests

3. **Lobby UI** (replaces "Start Game")
   - "Create Lobby" button
   - "Join Friend's Lobby" list
   - Lobby screen: members, ready status
   - "Ready" button
   - "Start" button (host only, when all ready)

4. **HTTP Client** (C++)
   - Use existing HttpClient
   - Add Bearer token to headers
   - Parse JSON responses

### Security Notes

- Passwords hashed with bcrypt (cost 10)
- Tokens are 32-byte cryptographically secure random
- Sessions expire after 7 days
- Auth middleware validates token + expiry on every request
- SQL injection protected (parameterized queries)

### Production Considerations

**Current (SQLite):**
- Good for: 100-1000 concurrent users
- Single file: `coordinator.db`
- No separate database server needed

**Future (PostgreSQL):**
- Needed for: 10,000+ concurrent users
- Horizontal scaling
- Replication/backups
- Just swap Database implementation

### Files Created/Modified

**New files:**
- `coordinator/database.go` - Database layer with SQLite
- `coordinator/types.go` - Data structures for auth/friends/lobby
- `coordinator/auth.go` - Authentication service
- `coordinator/friends.go` - Friends management
- `coordinator/lobby.go` - Lobby system
- `coordinator/test_new_api.sh` - API test script

**Modified files:**
- `coordinator/main.go` - Integrated all services
- `coordinator/go.mod` - Added dependencies (sqlite3, bcrypt)

**Dependencies added:**
- `github.com/mattn/go-sqlite3` - SQLite driver
- `golang.org/x/crypto` - bcrypt password hashing
