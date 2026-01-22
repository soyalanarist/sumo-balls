# Sumo Balls Coordinator (Go)

A lightweight matchmaking and game server orchestration service for Sumo Balls written in Go.

## Building

### Prerequisites
- Go 1.21 or later
- Linux, macOS, or Windows

### Compile
```bash
cd coordinator
go build -o coordinator-bin main.go
```

This creates a single static binary `coordinator-bin` with no external dependencies (Go standard library only).

## Running

```bash
./coordinator-bin
```

Server will listen on `http://localhost:8888` by default.

### Output
```
2024/01/15 10:23:45 [Coordinator] Starting on :8888
2024/01/15 10:23:47 [Coordinator] Player player123 enqueued (total: 1)
2024/01/15 10:23:48 [Coordinator] Queue status: 1 players waiting
```

## API Endpoints

### Client Endpoints

**POST /enqueue**
Enqueue a player for matchmaking.

Request:
```json
{
  "player_id": "player123",
  "mode": "matchmaking",
  "region": "us-west"
}
```

Response:
```json
{
  "queue_id": "q_player123",
  "status": "queued"
}
```

**GET /queue/status**
Check queue position and estimated wait time.

Query params: `player_id=player123`

Response:
```json
{
  "status": "queued",
  "elapsed_ms": 5000,
  "est_wait_ms": 12000
}
```

If match is found, response becomes:
```json
{
  "status": "match_found",
  "server_host": "192.168.1.100",
  "server_port": 9999,
  "token": "abcd1234efgh5678",
  "match_id": "match_1",
  "server_time_ms": 1234567890
}
```

**POST /cancel**
Remove player from queue.

Request:
```json
{
  "player_id": "player123"
}
```

Response:
```json
{
  "status": "canceled"
}
```

### Server Endpoints

**POST /server/register**
Register a game server with the coordinator.

Request:
```json
{
  "server_id": "server_us_west_1",
  "host": "192.168.1.100",
  "port": 9999,
  "max_players": 4
}
```

Response:
```json
{
  "status": "registered"
}
```

**POST /server/heartbeat**
Keep-alive ping from game server. Should be called every 5-10 seconds.

Request:
```json
{
  "server_id": "server_us_west_1"
}
```

Response:
```json
{
  "status": "heartbeat_received"
}
```

### Debug Endpoints

**GET /matches**
List all active matches (for debugging).

Response:
```json
[
  {
    "match_id": "match_1",
    "server_id": "server_us_west_1",
    "players": ["player123", "player456"],
    "created": "2024-01-15T10:23:50Z"
  }
]
```

## How It Works

1. **Client Enqueues**: Client calls `POST /enqueue` with player ID
2. **Matchmaking Loop**: Every 2 seconds, coordinator pulls 2+ players from queue and creates a match
3. **Server Assignment**: Coordinator picks least-loaded available server
4. **Token Generation**: Secure random token issued to players for session admission
5. **Client Polls**: Client calls `GET /queue/status` every 1 second
6. **Match Found**: When status returns `match_found`, client extracts server host/port/token
7. **Server Validates**: Game server checks token before accepting client join (planned integration)

## C++ Integration

### Client Side (C++)
```cpp
// When "Matchmaking" button clicked:
// POST http://localhost:8888/enqueue
// {
//   "player_id": playerId,
//   "mode": "matchmaking",
//   "region": "us-west"
// }

// Poll every 1 second:
// GET http://localhost:8888/queue/status?player_id=<playerId>

// When response has "match_found":
// Extract server_host, server_port, token
// Connect to game server with token
```

### Server Side (C++)
```cpp
// On startup:
// POST http://localhost:8888/server/register
// {
//   "server_id": "server_1",
//   "host": "192.168.1.100",
//   "port": 9999,
//   "max_players": 4
// }

// Every 5-10 seconds:
// POST http://localhost:8888/server/heartbeat
// {"server_id": "server_1"}

// When client joins:
// Validate token (token sent in JoinRequest message)
// Decode token to verify playerId and matchId match
```

## Architecture Notes

**Why Go?**
- **Goroutines**: Handle 1000s of concurrent queue checks efficiently (vs C++ threads which are heavyweight)
- **Static Binary**: Single file, no runtime dependencies, easy to deploy
- **Simplicity**: Network code is in standard library, no external deps needed
- **Observability**: Built-in HTTP profiling (`/debug/pprof`)

**Concurrency Model**
- `sync.Mutex` protects shared state (like C++ `std::mutex`)
- Goroutines are Go's concurrency primitive (similar to `std::thread` but lighter)
- Channels can be used for async communication between goroutines

**Future Improvements**
- Persistent storage (SQLite/PostgreSQL) instead of in-memory maps
- Redis for distributed coordinator (scale from 1 to N instances)
- Proper token validation with HMAC/JWT signatures
- Server health checks with configurable timeouts
- Region-based matchmaking (latency-aware server selection)
- Backfill (add players to matches in progress)
- Anti-cheat token validation with signing key
