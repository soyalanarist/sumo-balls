# Sumo Balls - Multiplayer Arena Game

A fast-paced multiplayer arena game where players try to knock each other out of a shrinking arena. Built with modern C++20, featuring authoritative server architecture, real-time networking, user authentication, and matchmaking.

## Features

- **Online Multiplayer**: Authoritative server with client-side prediction and server reconciliation
- **Smooth 60 FPS Gameplay**: Optimized networking with interpolation and low-latency updates
- **Physics-Based Combat**: Momentum-based collisions and realistic physics simulation
- **Shrinking Arena**: Dynamic arena that shrinks over time, intensifying gameplay
- **Particle Effects**: Visual feedback for explosions and collisions
- **Account System**: User registration, login, and session management
- **Friends & Lobbies**: Friend lists, friend requests, and private game lobbies
- **Friend Queueing**: Create parties with friends, queue together, auto-match and auto-create lobbies
- **Matchmaking**: Automatic queue-based match creation via coordinator service

## UI Architecture (Core, Views, Scenes)

To simplify and standardize navigation and UI, the client now uses a clear separation between core navigation, UI views, and gameplay scenes:

- **Core** (navigation & base types):
    - `src/core/Screen.h/.cpp` — base type with `update()`, `render()`, `handleInput()`, and `wantsInput()`
    - `src/core/ScreenStack.h/.cpp` — stack-based navigator with event bubbling and overlay support
    - `src/core/ScreenTransition.h` — enum of actions (e.g., `TO_MAIN_MENU`, `TO_MATCH`, `PAUSE`, `SET_FULLSCREEN`)

- **Views** (UI screens):
    - `src/ui/views/MainMenuView.*`, `PauseMenuView.*`, `SettingsView.*`, `LoginView.*`, `CreatePlayerView.*`, `FriendsView.*`, `LobbyView.*`, `MatchResultsView.*`
    - Views typically push transitions (e.g., pressing Play sets `ScreenTransition::TO_MATCH`)

- **Scenes** (gameplay):
    - `src/ui/scenes/MatchScene.*` — main gameplay
    - `src/ui/scenes/MatchResultsScene.*` — end-of-match summary overlay

### Navigation Flow

Screens communicate desired navigation via `ScreenTransition`. The `ScreenStack` polls the top screen, consumes the transition, and mutates the stack accordingly. Overlays (`isOverlay() == true`) render above non-overlays and participate in event bubbling while respecting `wantsInput()`.

### Rationale

- Removes the old “menus vs screens” duality in favor of a single base `Screen`
- Clarifies intent with `views` (UI) and `scenes` (gameplay)
- Centralizes transitions in core to make navigation robust and testable


## Quick Start (Fresh Clone)

```bash
# From repo root
chmod +x scripts/bootstrap.sh
./scripts/bootstrap.sh           # macOS or Debian/Ubuntu; installs deps and builds Debug
./scripts/run-coordinator.sh &   # starts auth/friends/lobby service on :8888
./scripts/run-server.sh 7777 &   # optional dedicated game server
./scripts/run-client.sh          # launches client; auto-builds if missing
```

- macOS: installs Homebrew if missing, then installs cmake, ninja, pkg-config, SDL2 (+image/ttf/mixer), Go.
- Debian/Ubuntu: installs build-essential, cmake, ninja, pkg-config, SDL2 dev libs, Go, clang-format/clang-tidy.
- ENet/ImGui are fetched automatically by CMake.

### CMake Presets
- Configure: `cmake --preset debug` (or `release`, `asan`)
- Build: `cmake --build --preset debug`

Navigate the menu with your mouse; click "Play" for local or online tests.

### Dev Stack with Docker (optional)

```bash
docker compose up coordinator        # starts coordinator on :8888 (with sqlite db persisted)
docker compose up server --profile server  # builds + runs dedicated server on :7777
docker compose up seed              # seeds two users (alice/bob) against coordinator
```

### Code Quality & CI

- CI: GitHub Actions builds on Ubuntu/macOS, runs C++ tests and Go tests.
- Formatting: `./scripts/format.sh` (clang-format for C++, gofmt for Go). Install pre-commit and run `pre-commit install` to auto-format.
- Linting: clang-tidy/clang-format and gofmt available via bootstrap and CI.

### Testing

**C++ Tests** (Physics, Utilities)
```bash
cd build
ctest --output-on-failure
# or
./sumo_balls_test
```

**Go Tests** (Auth, Friends, Lobbies)
```bash
cd coordinator
go test -v ./...
```

**Linting & Formatting**
```bash
# Check code style
./scripts/format.sh --check

# Auto-format code
./scripts/format.sh

# Coordinator linting (requires golangci-lint)
cd coordinator && golangci-lint run

# C++ static analysis (requires clang-tidy)
clang-tidy src/**/*.cpp --checks=-*,readability-*,performance-*,bugprone-*
```

### Coordinator Config

- Env vars: `PORT` (default 8888), `DB_PATH` (default coordinator.db), `GOOGLE_CLIENT_ID`, `GOOGLE_CLIENT_SECRET`.
- Sample: copy `.env.example`, export it, then run `./scripts/run-coordinator.sh`.

---

## Building the Game

### Supported Platforms

- **Linux** (Ubuntu 20.04+, Debian 11+, Arch)
- **macOS** (10.13+)
- **Windows** (via WSL2 or MSVC with SDL2 port)

### Install Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev
```

#### Arch Linux

```bash
sudo pacman -S base-devel cmake sdl2
```

#### macOS

```bash
brew install cmake sdl2
```

### Build Instructions

```bash
# Clone repository
git clone <repo>
cd sumo-balls

# Create build directory
mkdir -p build
cd build

# Configure (choose one)
cmake -DCMAKE_BUILD_TYPE=Debug ..     # Debug with sanitizers
cmake -DCMAKE_BUILD_TYPE=Release ..   # Optimized release build

# Compile
make -j$(nproc)
```

**Output Executables:**
- `sumo_balls` - Game client
- `sumo_balls_server` - Dedicated game server
- `sumo_balls_test` - Unit test suite

### Running Tests

```bash
cd build
./sumo_balls_test
```

Example output:
```
======================================================================
Running 5 test(s)
======================================================================

✓ Physics::FiniteCheck                                         [0.00ms]
✓ Physics::PositionValidation                                  [0.00ms]
✓ Physics::VelocityValidation                                  [0.00ms]
✓ Physics::PositionClamping                                    [0.00ms]
✓ Physics::VelocityClamping                                    [0.00ms]

======================================================================
Results: 5 passed, 0 failed
======================================================================
```

---

## Running the Game

### Local/Offline Mode

Perfect for single-player testing or LAN parties without internet:

```bash
cd build
./sumo_balls
```

**Controls:**
- **WASD** - Movement
- **Arrow Keys** - Alternative movement
- **IJKL** - Left-handed mode (enable in settings)
- **ESC** - Pause/Menu
- **Mouse** - Menu navigation

### Online Mode with Full Stack

Requires three services running: Coordinator, Game Server, and Client.

#### Step 1: Start Coordinator (Matchmaking Service)

```bash
cd coordinator
go build -o coordinator-bin main.go
./coordinator-bin
```

Expected output:
```
[Coordinator] Starting on :8888
[Auth] User database initialized
```

This provides:
- User authentication (register/login)
- Session management
- Friends system
- Lobby management
- Matchmaking queue

#### Step 2: Start Game Server(s)

```bash
cd build
./sumo_balls_server 9999
```

Expected output:
```
[Server] Registered with coordinator
[Server] Listening on port 9999
[Server] Snapshot rate: 33 Hz (30ms)
[Server] Simulation tick: 2ms
```

You can run multiple servers for load balancing.

#### Step 3: Launch Client(s)

```bash
cd build
./sumo_balls
```

Flow:
1. Click "Login" or "Register" to create account
2. Upon authentication, click "Matchmaking"
3. Game automatically finds opponents
4. Coordinator creates match when sufficient players queued
5. Client connects to assigned game server
6. Match begins!

---

## Configuration

### Client Config (`config.json`)

Located in game root directory:

```json
{
  "leftyMode": false,
  "fullscreen": false,
  "playerColorIndex": 5,
  "onlineEnabled": true,
  "onlineHost": "localhost",
  "onlinePort": 9999
}
```

**Options:**
- `leftyMode` (bool): Use IJKL instead of WASD
- `fullscreen` (bool): Run in fullscreen mode
- `playerColorIndex` (int): Player color (0-7)
- `onlineEnabled` (bool): Enable online features
- `onlineHost` (string): Coordinator/server IP address
- `onlinePort` (int): Server port

### Server Configuration

Edit constants in `src/server_main.cpp`:

```cpp
const float snapshotInterval = 0.03f;  // 30ms between state broadcasts
const int   simulationTickMs = 2;      // 2ms per physics simulation step
const int   maxPlayersPerServer = 6;   // Maximum concurrent players
```

### Authentication

The game supports three authentication methods:

#### 1. Username/Password (Default)

No configuration needed. Works offline if coordinator unavailable.

#### 2. Google OAuth (Requires Setup)

Allows users to sign in with their Google account. Requires credentials from Google Cloud Console.

**Quick Setup (WSL + Windows 11):**

```bash
# 1. Set up OAuth credentials
bash scripts/setup-oauth.sh

# 2. Start coordinator (loads credentials)
cd coordinator && ./run.sh

# 3. Run game client
cd build && ./sumo_balls

# Click "Sign in with Google" - browser opens automatically via wslview
```

**Full Documentation:**
- [Google OAuth Setup Guide](docs/GOOGLE_OAUTH_SETUP.md) - Detailed steps with Google Cloud Console instructions
- [WSL Browser Guide](docs/WSL_BROWSER_GUIDE.md) - Troubleshooting for WSL → Windows browser opening

**WSL Automatic Browser Opening:**

When you click "Sign in with Google" on WSL, the game automatically:
1. Detects WSL environment
2. Uses `wslview` to open browser in Windows
3. Handles OAuth redirect back to localhost:8888
4. Polls for completion (30 second timeout)
5. Logs you in automatically

No manual browser URL copying needed!

**Environment Variables:**

```bash
export GOOGLE_CLIENT_ID="your_client_id_from_google_cloud"
export GOOGLE_CLIENT_SECRET="your_client_secret_from_google_cloud"
./coordinator-bin
```

The coordinator also manages:
- SQLite user database (auto-created)
- Session tokens with 7-day expiry
- Secure password hashing (bcrypt)

---

## Network Architecture

### Protocol & Requirements

| Aspect | Details |
|--------|---------|
| Protocol | UDP via ENet (reliable delivery) |
| Server Tick | 500 Hz (2ms per step) |
| Snapshot Rate | 33 Hz (30ms broadcast interval) |
| Server Bandwidth | ~10 KB/s per player (upstream) |
| Client Bandwidth | ~5 KB/s (downstream) |
| Optimal Latency | <100ms RTT |
| Max Playable | <150ms RTT |

### Server Architecture

- **Authoritative**: Server is single source of truth
- **Deterministic**: Same inputs produce same outputs
- **Stateless players**: Server doesn't care about client state
- **Snapshots**: Periodic full-state broadcasts every 30ms

### Client Architecture

- **Prediction**: Client predicts own movement before server confirmation
- **Interpolation**: Smoothly interpolates between received snapshots
- **Reconciliation**: Corrects deviations when server state differs
- **Input buffering**: Handles 50-100ms RTT transparently

### Port Forwarding (For Internet Play)

If hosting from home network, forward UDP port on your router:

1. Access router admin panel (usually 192.168.1.1)
2. Find "Port Forwarding" or "UPnP" settings
3. Add rule: `External UDP Port 9999 → Internal IP:9999`
4. Share your public IP with players

Find your public IP:
```bash
curl ifconfig.me
```

---

## Deployment

### Local Network (LAN)

```bash
# Terminal 1: Coordinator
cd coordinator && ./coordinator-bin

# Terminal 2: Server
cd build && ./sumo_balls_server 9999

# Terminal 3+: Clients
cd build && ./sumo_balls
```

### Internet Play (Single Server)

```bash
# On server machine with public IP or port forwarding:
cd coordinator && ./coordinator-bin

# In separate terminal:
cd build && ./sumo_balls_server 9999

# Clients update config.json:
# "onlineHost": "your.public.ip"
```

### Cloud Deployment (VPS/AWS/DigitalOcean)

```bash
# SSH into Ubuntu VPS
ssh user@your-vps-ip

# Install dependencies
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev golang-go

# Clone and build
git clone <repo>
cd sumo-balls
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

# Create systemd service for game server
sudo tee /etc/systemd/system/sumo-balls.service > /dev/null << EOF
[Unit]
Description=Sumo Balls Game Server
After=network.target

[Service]
Type=simple
User=gameserver
WorkingDirectory=/opt/sumo-balls/build
ExecStart=/opt/sumo-balls/build/sumo_balls_server 9999
Restart=on-failure
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

# Enable and start
sudo systemctl daemon-reload
sudo systemctl enable sumo-balls
sudo systemctl start sumo-balls

# View logs
sudo journalctl -u sumo-balls -f
```

---

## Project Architecture

### Directory Structure

```
sumo-balls/
├── src/
│   ├── main.cpp              # Client entry point
│   ├── server_main.cpp       # Server entry point
│   ├── core/                 # Engine (graphics, screens, ImGui)
│   │   ├── Game.h/cpp        # Main game loop
│   │   ├── GraphicsContext   # SDL2 wrapper
│   │   ├── ImGuiManager      # ImGui initialization
│   │   ├── ScreenStack       # Menu navigation
│   │   └── Settings          # Config management
│   ├── game/                 # Game logic
│   │   ├── entities/         # Players, arena, effects
│   │   ├── PhysicsValidator  # Physics safety checks
│   │   └── controllers/      # Input handling (human, AI, network)
│   ├── network/              # Networking
│   │   ├── NetClient         # ENet client wrapper
│   │   ├── NetServer         # ENet server wrapper
│   │   ├── NetProtocol       # Message serialization
│   │   ├── HttpClient        # Coordinator API calls
│   │   └── GameNetworkManager# Connection lifecycle
│   ├── screens/              # UI/Game screens
│   │   ├── AuthScreen        # Login/Register UI
│   │   ├── GameScreen        # Main gameplay
│   │   ├── FriendsScreen     # Friends list
│   │   ├── LobbyScreen       # Lobby UI
│   │   └── menus/            # Main menu, pause, game-over
│   ├── simulation/           # Physics engine
│   │   └── Simulation        # Collision, movement, arena shrink
│   ├── systems/              # Game systems
│   │   └── InputSystem       # Keyboard input polling
│   └── utils/                # Utilities
│       ├── VectorMath        # Vector2 implementation
│       ├── GameConstants     # Shared constants
│       ├── SimpleJson        # JSON parsing
│       └── Logger            # Logging system
├── coordinator/              # Go matchmaking service
│   ├── main.go               # Server, queue, matchmaking
│   ├── auth.go               # User auth, sessions
│   ├── database.go           # SQLite operations
│   ├── friends.go            # Friend requests/list
│   ├── lobby.go              # Lobby management
│   └── types.go              # API request/response types
├── tests/                    # Unit tests
│   ├── TestFramework.h       # Simple test macro system
│   ├── TestRunner.cpp        # Test main()
│   └── test_physics.cpp      # Physics validation tests
├── CMakeLists.txt            # C++ build configuration
├── .clang-format             # Code style configuration
├── config.json               # Default client config
└── README.md                 # This file
```

### Design Patterns

**Architectural Patterns:**
- **Client-Server**: Centralized authority
- **Screen Stack**: Menu navigation via push/pop
- **Event-Driven**: Network events trigger state changes
- **Manager Pattern**: Singleton managers for graphics, audio, network

**Code Patterns:**
- **RAII**: Resource management (smart pointers, destructors)
- **Observer**: UI updates from game state changes
- **Strategy**: Different controllers (human, AI, network)
- **Factory**: Entity creation with appropriate components

### Technologies & Libraries

| Component | Library | Purpose |
|-----------|---------|---------|
| Graphics | SDL2 | Cross-platform windowing, rendering |
| UI | ImGui | Immediate-mode GUI for menus |
| Networking | ENet | Reliable UDP with flow control |
| Physics | Custom | Collision detection, impulse resolution |
| Math | GLM-inspired | Vector math utilities |
| Serialization | SimpleJson | Lightweight JSON for coordinator API |
| Backend | Go + SQLite | Matchmaking, auth, friends, lobbies |

---

## Development

### Code Standards

**Language**: C++20

**Style**: 
- Run `clang-format` on all files
- Config: `.clang-format` file included

**Naming**:
- `snake_case` for functions and variables
- `PascalCase` for classes and types
- `UPPER_CASE` for constants

**Error Handling**:
- Exceptions for fatal/unrecoverable errors
- Logging for diagnostics and debugging
- PhysicsValidator for sanity checks

**Comments**:
- Document public APIs with intention
- Explain "why", not "what"
- Use modern C++ constructs, not C-style comments

### Building with Format Check

```bash
# Auto-format all source files
clang-format -i src/**/*.cpp include/**/*.h

# Or integrate with editor (VS Code config provided)
```

### Debug vs Release Builds

**Debug Build** (`-DCMAKE_BUILD_TYPE=Debug`)
- Address Sanitizer enabled (detects memory errors)
- Undefined Behavior Sanitizer enabled
- Debug symbols for debugging
- Slower execution but catches errors

**Release Build** (`-DCMAKE_BUILD_TYPE=Release`)
- `-O3` optimizations enabled
- `-march=native` for CPU-specific optimizations
- No sanitizers (production-safe)
- ~2-3x faster execution

### Running with Sanitizers

```bash
# Build in Debug mode
cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make

# Run normally - sanitizers run automatically
./sumo_balls

# Output shows errors like:
# ==12345==ERROR: AddressSanitizer: heap-buffer-overflow
# ==12345==The signal is caused by READ memory access
# ==12345==Hint: address points to the heap block at ...
```

### Writing Tests

Add test functions to `tests/test_*.cpp`:

```cpp
#include "TestFramework.h"
#include "../src/mymodule.h"

bool testMyFeature(std::string& errorMsg) {
    // Arrange - set up test data
    int expected = 42;
    
    // Act - perform the operation
    int actual = myFunction(100);
    
    // Assert - verify result
    TEST_EQUAL(expected, actual, "Function should return 42");
    return true;
}

// Register test
namespace {
    struct MyTestSuite {
        MyTestSuite() {
            test::TestSuite::instance().registerTest(
                "MyModule::TestFeature", 
                testMyFeature
            );
        }
    } myTests;
}
```

Build and run:
```bash
cd build && make && ./sumo_balls_test
```

---

## Troubleshooting

### Build Issues

**"SDL2 not found"**
```bash
sudo apt-get install libsdl2-dev
```

**"CMake version too old"**
```bash
pip install --upgrade cmake
# Or download from cmake.org
```

**"Permission denied" on scripts**
```bash
chmod +x scripts/*.sh
```

### Runtime Issues

**"Connection refused" / Cannot connect**
```bash
# Verify coordinator running
lsof -i :8888

# Verify server running
lsof -i :9999

# Check firewall
sudo ufw status
sudo ufw allow 8888/tcp
sudo ufw allow 9999/udp
```

**"Players not visible" / Connection drops**
- Both clients must connect to same game server
- Check RTT display in-game (should be <150ms)
- Verify packet loss: `ping -c 10 server-ip`

**"High CPU usage"**
- Check if using Release build (`-DCMAKE_BUILD_TYPE=Release`)
- Profile with: `perf record -g ./sumo_balls && perf report`
- Reduce snapshot rate if CPU-bound on server

**Sanitizer reports issues**
```
==123==ERROR: AddressSanitizer: use-after-free
```

This indicates a real bug. Check the stack trace:
- Shows exact function and line where error occurs
- Trace back to find where object was freed prematurely

---

## Performance

### Typical Metrics

| Metric | Value |
|--------|-------|
| Server CPU per player | 1-2% |
| Client CPU | 5-10% |
| Client Memory | <50 MB |
| Server Memory | <100 MB |
| Network (server) | 10 KB/s |
| Network (client) | 5 KB/s |
| Latency tolerance | <150ms RTT |
| Frame time | 16.7ms @ 60 FPS |

### Optimization Tips

- **Server**: Reduce `snapshotInterval` for lower latency (higher bandwidth)
- **Client**: Increase interpolation smoothness by reducing `frameTime`
- **Network**: Compress physics state if bandwidth-limited
- **Physics**: Cache collision shapes, use spatial partitioning

---

## Contributing

1. **Code Style**: Run `clang-format` on modified files
2. **Testing**: Add tests for new features, run `sumo_balls_test`
3. **Documentation**: Update README and inline comments
4. **Commits**: Clear messages explaining "why"
5. **Pull Requests**: Include test results and performance impact

---

## License

[Add your license here]

## Credits

[Add credits here]

## Additional Resources

For more details:
- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - Deep dive into system design
- [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) - Development status
- [COORDINATOR_INTEGRATION.md](docs/COORDINATOR_INTEGRATION.md) - Matchmaking API

---

**Last Updated**: January 2026  
**Version**: 0.1.0 (Alpha)
