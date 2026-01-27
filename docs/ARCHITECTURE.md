# Sumo Balls: Complete Architecture Guide

## System Overview

Sumo Balls is a multiplayer physics-based game with distributed architecture:
- **Client**: C++20 with SDL2/ImGui (60 FPS, client-side prediction, server reconciliation)
- **Game Server**: C++20 authoritative simulation (500Hz internal, 33Hz snapshot rate, ENet UDP)
- **Coordinator**: Go 1.21 service (HTTP REST API, auth, friends, lobbies, SQLite DB)

### High-Level Topology

```
┌─────────────────────────────────────┐      ┌─────────────────────┐
│      Game Client (C++20/SDL2)       │      │  Game Server (C++)  │
│  ┌───────────────────────────────┐  │      │  ┌───────────────┐  │
│  │ UI/Screens (ImGui)            │  │      │  │ NetServer(ENet)   │
│  │ - Auth, Lobbies, Friends      │  │      │  │ Port 7777     │  │
│  └───────────────────────────────┘  │      │  └───────────────┘  │
│  ┌───────────────────────────────┐  │      │  ┌───────────────┐  │
│  │ SocialManager (HTTP)          │◄─┼─────┼─►│ Simulation    │  │
│  │ - Auth, Friends, Lobbies      │  │ JSON│  │ - Authoritative   │
│  └───────────────────────────────┘  │      │  │ - 500Hz tick  │  │
│  ┌───────────────────────────────┐  │      │  └───────────────┘  │
│  │ NetClient (ENet)              │◄─┼─────►│ Physics       │  │
│  │ Port 7777                     │  │ UDP  │ - Player motion    │
│  └───────────────────────────────┘  │      │ - Collision      │  │
│  ┌───────────────────────────────┐  │      └─────────────────────┘
│  │ Simulation (Client-side)      │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
         HTTP (port 8888) ▲
         JSON over REST    │
┌─────────────────────────────────────┐
│   Coordinator (Go/SQLite)           │
│   Port 8888                         │
├─────────────────────────────────────┤
│ Auth Service      │ Friends Service │
│ - Register/Login  │ - Send request  │
│ - OAuth2          │ - List friends  │
├─────────────────────────────────────┤
│ Lobby Service  │ Database (SQLite) │
│ - Create/Join  │ - Users, Sessions │
│ - List lobbies │ - Friends, Lobbies│
└─────────────────────────────────────┘
```

## Quick Start

### 1. Build Everything

```bash
# Build C++ client and server
./build.sh

# Build Go coordinator
cd coordinator && go build -o coordinator-bin main.go && cd ..
```

### 2. Run Local Test (3 terminals)

**Terminal 1: Coordinator**
```bash
cd coordinator
./coordinator-bin
# Output: [Coordinator] Starting on :8888
```

**Terminal 2: Game Server**
```bash
./run-server.sh
# Output: [Server] Registered with coordinator
#         [Server] Listening on port 9999
```

**Terminal 3 & 4: Clients**
```bash
./run-client.sh
# Click "Matchmaking" in menu
```

Expected behavior:
- Both clients appear in queue
- After ~2 seconds, coordinator creates match
- Clients receive server address and token
- Clients connect to server and join game

## Project Structure

```
sumo-balls/
├── CMakeLists.txt              # C++ build config (client + server)
├── src/
│   ├── main.cpp                # Client entry point (SFML window, game loop)
│   ├── server_main.cpp         # Server entry point (ENet listening, snapshot broadcast)
│   ├── core/
│   │   ├── Game.h/cpp          # Game state manager
│   │   ├── Screen.h/cpp        # Screen base class
│   │   ├── ScreenStack.h/cpp   # Menu/game screen navigation
│   │   └── Settings.h/cpp      # Config loading from JSON
│   ├── game/
│   │   ├── entities/           # Players, balls, physics
│   │   └── controllers/        # Input handling (keyboard, network)
│   ├── network/
│   │   ├── NetProtocol.h       # Message types (JoinRequest, State, etc.)
│   │   ├── NetClient.h/cpp     # ENet client (connect, send/recv, events)
│   │   ├── NetServer.h/cpp     # ENet server (listen, accept, broadcast)
│   │   └── NetCommon.h/cpp     # Shared serialization helpers
│   ├── screens/
│   │   ├── GameScreen.h/cpp    # Main game screen (prediction, interp, server recon)
│   │   └── menus/              # Main menu, settings, queue UI
│   ├── simulation/
│   │   ├── Simulation.h/cpp    # Physics tick (2ms), collision detection
│   ├── systems/
│   │   └── InputSystem.h/cpp   # Keyboard input (arrow keys, WASD)
│   ├── ui/
│   │   ├── Button.h/cpp        # Menu buttons
│   │   ├── UIElement.h/cpp     # Base UI class
│   │   └── ToggleSwitch.h/cpp  # Settings toggles
│   └── utils/
│       ├── GameConfig.h        # Config struct (JSON deserialization)
│       └── VectorMath.h        # Math utilities (interpolation, blending)
├── include/                    # External headers (SFML, ENet)
├── build/                      # Build artifacts (created by CMake)
│   ├── sumo_balls              # Client executable
│   └── sumo_balls_server       # Server executable
├── coordinator/                # Go matchmaking service
│   ├── main.go                 # HTTP server, queue manager, server registry
│   ├── go.mod                  # Go module config
│   ├── coordinator-bin         # Go executable (created by `go build`)
│   └── README.md               # Coordinator API docs
├── config.json                 # Game settings (server host/port, online mode, etc.)
├── README.md                   # Build & run instructions
├── DEPLOYMENT.md               # LAN/Internet/VPS deployment guide
├── COORDINATOR_INTEGRATION.md  # How to integrate coordinator with C++ code
└── .devcontainer/              # VS Code dev container for Mac/WSL sync
```

## Network Architecture

### Transport Layers

**Layer 1: Coordinator ↔ Client/Server (HTTP REST)**
- Technology: Go HTTP server (port 8888)
- Format: JSON
- Purpose: Queue management, match allocation, token generation
- Endpoints:
  - `POST /enqueue` - Client joins queue
  - `GET /queue/status` - Client polls for match
  - `POST /cancel` - Client leaves queue
  - `POST /server/register` - Server registers itself
  - `POST /server/heartbeat` - Server keep-alive ping

**Layer 2: Game Server ↔ Client (ENet UDP)**
- Technology: ENet 1.3 reliable/unreliable UDP
- Format: Binary (custom serialization)
- Purpose: Game state synchronization
- Messages:
  - `JoinRequest` (client → server) - Player joins game with session token
  - `JoinAccept` (server → client) - Confirm join, assign player ID
  - `Input` (client → server) - Player movement/actions
  - `State` (server → client) - World snapshot (30ms, 33 Hz)
  - `Ping/Pong` (both) - Latency measurement, keep-alive

### Session Flow

```
1. Client clicks "Matchmaking"
   ↓
2. Client: HTTP POST /enqueue → Coordinator
   ↓
3. Coordinator: Queue player, wait for 2+ players
   ↓
4. Coordinator: CREATE MATCH after ~2 seconds
   ↓
5. Client: HTTP GET /queue/status (polling) → Coordinator
   ↓
6. Coordinator: Response includes server host/port/token
   ↓
7. Client: ENet connect to server with session token
   ↓
8. Server: Validate token, accept JoinRequest
   ↓
9. Server: Broadcast game state to all players
   ↓
10. Client: Predict self, interpolate others, render
```

## Client-Side Networking

### Connection Flow ([src/screens/GameScreen.cpp](src/screens/GameScreen.cpp))
1. Parse server address from coordinator match response
2. `netClient->connect(host, port)` - Non-blocking ENet connect
3. Send `JoinRequest` with session token
4. Wait for `JoinAccept` (assigns player ID)
5. Start receiving `State` snapshots

### Input Handling ([src/systems/InputSystem.cpp](src/systems/InputSystem.cpp))
1. Poll keyboard every frame
2. Generate `Input` message (position delta)
3. Apply locally immediately (client-side prediction)
4. Send to server (unreliable, 60 FPS input rate)

### State Synchronization
- **Receive**: `State` snapshot from server every 30ms (33 Hz)
- **Interpolate**: Smoothstep interpolation for other players (0ms delay)
- **Self**: Use client-side prediction, blend with server reconciliation (35% server)
- **Tick**: Local physics simulation runs every 2ms (500 Hz theoretical)

## Server-Side Networking

### Connection Flow ([src/server_main.cpp](src/server_main.cpp))
1. Register with coordinator: `POST /server/register`
2. Listen on ENet port 9999
3. Accept client connections, validate session tokens
4. Broadcast snapshots every 30ms to all connected clients
5. Heartbeat coordinator every 5 seconds: `POST /server/heartbeat`

### Server Loop
```
while (running) {
    // Update physics simulation (tick every 2ms)
    simulation.tick(0.002f);
    
    // Check for client input and apply
    while (auto input = netServer.recvInput()) {
        applyPlayerInput(input);
    }
    
    // Broadcast state snapshot every 30ms (if time elapsed)
    if (timeSinceLastSnapshot >= 0.03f) {
        netServer.broadcastState(simulation.getWorldState());
        timeSinceLastSnapshot = 0;
    }
}
```

## Physics & Simulation

### Tick Rate: 2ms (~500 Hz)
- Fixed timestep for deterministic physics
- Collision detection via AABB + circle overlaps
- Gravity (downward), friction (player movement dampening)

### Client-Side Prediction
- Player's own movement: Apply input immediately without waiting for server
- Other players: Wait for server snapshot, then interpolate to smooth motion

### Server Reconciliation
- Server sends "ground truth" position every 30ms
- Client blends: `displayPos = lerp(predictedPos, serverPos, 0.35)`
- If large discrepancy, snap to server position

## Configuration

### [config.json](config.json)
```json
{
  "onlineHost": "localhost",      // Default: localhost for testing
  "onlinePort": 9999,             // Game server port
  "onlineEnabled": false,         // Set to true to enable online mode
  "leftyMode": false,             // Arrow keys: false, IJKL: true
  "coordinatorUrl": "http://localhost:8888"  // Coordinator address
}
```

## Build System

### C++ (CMake 3.16+)
```bash
cmake -B build
cmake --build build
```

Targets:
- `sumo_balls` - Client executable
- `sumo_balls_server` - Server executable

### Go (Go 1.21+)
```bash
cd coordinator
go build -o coordinator-bin main.go
```

Single static binary, no runtime dependencies.

## Deployment

### Local Development
- All three services (coordinator, server, client) on same machine
- Use `localhost:8888` (coordinator), `localhost:9999` (server)

### LAN Testing (3 machines)
- Coordinator on PC1 (192.168.1.10:8888)
- Server on PC2 (192.168.1.20:9999)
- Clients on PC2, PC3, MacBook
- Update `config.json`: `onlineHost: "192.168.1.20"`, `coordinatorUrl: "http://192.168.1.10:8888"`

### Internet Deployment
- Coordinator on cloud VPS (e.g., AWS t2.micro)
- Multiple game servers on regional VPCs
- Clients connect via public IPs
- Use HTTPS, add authentication layer

See [DEPLOYMENT.md](DEPLOYMENT.md) for detailed instructions.

## Performance Benchmarks

### Client
- Frame rate: 60 FPS (capped), average frame time: 16.6ms
- Memory: ~150MB (SFML textures + game state)
- Network: ~2 KB/s (input) + ~5 KB/s (snapshots)
- Input latency: ~33ms (client prediction hides this)

### Server
- Tick rate: 500 Hz (2ms per tick, sleeps for jitter)
- CPU: <1% (small physics world, 2 players)
- Memory: ~50MB (ENet buffers, player states)
- Snapshots: 30ms interval, 33 Hz broadcast

### Coordinator
- Concurrent connections: Can handle 1000+ (Go goroutines are light)
- Matchmaking latency: ~2 seconds (hardcoded to wait for 2 players)
- CPU: <1% (mostly idle, matches every 2 sec)
- Memory: ~10MB (in-memory maps)

## Known Limitations & TODOs

### Current Limitations
- ❌ Collisions "don't go through" - reported but not yet diagnosed
- ❌ Both clients on same machine share keyboard (SFML limitation)
- ❌ Token generation is random (not cryptographically validated)
- ❌ No disconnect handling (clients must manually reconnect)
- ❌ Queue timeout (players wait indefinitely if server is down)

### Future Enhancements
1. **Token Signing**: HMAC-SHA256 signature for secure validation without coordinator
2. **Backfill**: Add players to matches in progress
3. **Region Latency**: Route clients to nearest server
4. **Persistence**: Move from in-memory to database (SQLite/PostgreSQL)
5. **Distributed Coordinator**: Redis-backed state for multiple coordinators
6. **Observability**: Prometheus metrics (queue length, match creation rate, player count)
7. **Anti-Cheat**: Validate client inputs on server, detect aimbots/speed hacks
8. **Matchmaking Skill**: ELO-based rating system for balanced teams

## Code Examples

### Enqueue (Client C++)
```cpp
void GameScreen::enqueueForMatchmaking() {
    CURL* curl = curl_easy_init();
    json req = {
        {"player_id", playerId},
        {"mode", "matchmaking"},
        {"region", "us-west"}
    };
    
    std::string body = req.dump();
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8888/enqueue");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    pollTimer = 0;  // Start polling for match
}
```

### Register Server (Server C++)
```cpp
void serverMain() {
    std::string serverId = "server_west_1";
    json req = {
        {"server_id", serverId},
        {"host", "192.168.1.100"},
        {"port", 9999},
        {"max_players", 4}
    };
    
    coordinatorPost("/server/register", req);
    // ... start game loop ...
}
```

### Queue Manager (Go Coordinator)
```go
func (c *Coordinator) Enqueue(playerID string) string {
    c.mu.Lock()
    defer c.mu.Unlock()
    
    queueID := fmt.Sprintf("q_%s", playerID)
    c.queuedPlayers = append(c.queuedPlayers, QueuedPlayer{
        PlayerID:   playerID,
        EnqueuedAt: timeNowMs(),
    })
    c.playerQueues[playerID] = queueID
    
    return queueID
}
```

## Troubleshooting

**Game server not found**
- Check coordinator is running: `curl http://localhost:8888/matches`
- Check server registered: Should see entry in matches list

**Client stuck in queue**
- Check server has capacity: `curl http://localhost:8888/matches | python3 -m json.tool`
- Check both players enqueued: Look for 2+ entries in queue

**Disconnected mid-game**
- Server crashed: Check `sumo_balls_server` logs
- Network timeout: ENet times out peers after 5 seconds no data
- Firewall: Check UDP port 9999 is open

**High latency / laggy movement**
- Reduce interpolation delay: Change `INTERPOLATION_DELAY_MS` in GameScreen.cpp
- Check frame rate: Should be 60 FPS; reduce graphics quality if needed
- Check network: `ping` server latency

## Further Reading

- [README.md](README.md) - Build and run instructions
- [DEPLOYMENT.md](DEPLOYMENT.md) - Production deployment guide
- [COORDINATOR_INTEGRATION.md](COORDINATOR_INTEGRATION.md) - Detailed integration steps
- [coordinator/README.md](coordinator/README.md) - Coordinator API reference
- ENet Documentation: http://enet.bespin.org/
- SFML Documentation: https://www.sfml-dev.org/documentation/
- Go Documentation: https://golang.org/doc/
