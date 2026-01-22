# Project Structure

This document describes the organization of the Sumo Balls codebase.

## Directory Layout

```
sumo-balls/
├── README.md                   # Main project documentation
├── CMakeLists.txt              # Build configuration
├── .gitignore                  # Git ignore patterns
├── config.json                 # Default game configuration
│
├── assets/                     # Game assets
│   └── arial.ttf              # Font file
│
├── build/                      # Build artifacts (gitignored)
│   ├── sumo_balls             # Game client executable
│   ├── sumo_balls_server      # Dedicated server executable
│   ├── test_physics           # Physics validation tests
│   └── test_ui                # UI component tests
│
├── config/                     # Configuration files
│   ├── config_client1.json    # Test client 1 config
│   ├── config_client2.json    # Test client 2 config
│   └── test_config.json       # Test configuration
│
├── coordinator/                # Matchmaking coordinator (Go)
│   ├── main.go                # Coordinator server
│   ├── go.mod                 # Go dependencies
│   ├── run.sh                 # Start coordinator
│   ├── test_api.sh            # API tests
│   └── README.md              # Coordinator documentation
│
├── docs/                       # Documentation
│   ├── ARCHITECTURE.md        # System architecture
│   ├── CODE_REVIEW.md         # Code review findings
│   ├── CODE_REVIEW_IMPLEMENTATION.md  # Improvements made
│   ├── COORDINATOR_INTEGRATION.md     # Matchmaking integration
│   ├── DEPLOYMENT.md          # Deployment guide
│   ├── IMPROVEMENTS.md        # Potential improvements
│   ├── MATCHMAKING_GUIDE.md   # Matchmaking usage
│   └── notes.txt              # Development notes
│
├── logs/                       # Log files (gitignored)
│   ├── client1.log
│   ├── client2.log
│   ├── coordinator.log
│   └── server.log
│
├── scripts/                    # Utility scripts
│   ├── build.sh               # Build the project
│   ├── run-server.sh          # Start game server
│   ├── run-client.sh          # Start game client
│   ├── test_online.sh         # Test online mode
│   ├── register-test-server.sh         # Register test server
│   ├── run-matchmaking-test.sh         # Test matchmaking
│   ├── run-matchmaking-gameplay-test.sh  # Full matchmaking test
│   └── commit-coordinator.sh  # Commit coordinator changes
│
├── src/                        # Source code
│   ├── main.cpp               # Client entry point
│   ├── server_main.cpp        # Server entry point
│   │
│   ├── core/                  # Core engine systems
│   │   ├── Game.h/.cpp        # Main game loop
│   │   ├── Screen.h/.cpp      # Screen base class
│   │   ├── ScreenStack.h/.cpp # Screen management
│   │   ├── Settings.h/.cpp    # Settings (legacy wrapper)
│   │   └── SettingsManager.h/.cpp  # Settings management
│   │
│   ├── game/                  # Game logic
│   │   ├── GamePhase.h        # Game state machine
│   │   ├── GamePhysics.h/.cpp # Physics and collisions
│   │   ├── PhysicsValidator.h # Physics validation utilities
│   │   │
│   │   ├── controllers/       # Input controllers
│   │   │   ├── Controller.h   # Controller interface
│   │   │   ├── HumanController.h/.cpp  # Player input
│   │   │   ├── AIController.h/.cpp     # AI opponents
│   │   │   └── NetworkController.h     # Network (TODO)
│   │   │
│   │   └── entities/          # Game entities
│   │       ├── Player.h/.cpp  # Player data
│   │       ├── PlayerEntity.h/.cpp  # Player with physics
│   │       └── Arena.h/.cpp   # Arena geometry
│   │
│   ├── network/               # Networking
│   │   ├── NetCommon.h/.cpp   # Network protocol
│   │   ├── NetClient.h/.cpp   # Client networking
│   │   ├── NetServer.h/.cpp   # Server networking
│   │   ├── NetProtocol.h      # Message definitions
│   │   ├── HttpClient.h       # HTTP client for matchmaking
│   │   └── GameNetworkManager.h/.cpp  # High-level network manager
│   │
│   ├── screens/               # Game screens
│   │   ├── GameScreen.h/.cpp  # Main gameplay
│   │   ├── GameEndedScreen.h/.cpp  # Game over overlay
│   │   │
│   │   └── menus/             # Menu screens
│   │       ├── MainMenu.h/.cpp
│   │       ├── OptionsMenu.h/.cpp
│   │       ├── PauseMenu.h/.cpp
│   │       └── GameOverMenu.h/.cpp
│   │
│   ├── simulation/            # Server-side simulation
│   │   └── Simulation.h/.cpp  # Authoritative physics
│   │
│   ├── systems/               # Game systems
│   │   └── InputSystem.h/.cpp # Input handling
│   │
│   ├── ui/                    # UI components
│   │   ├── UIElement.h/.cpp   # UI base class
│   │   ├── Button.h/.cpp      # Button widget
│   │   ├── ToggleSwitch.h/.cpp  # Toggle switch widget
│   │   └── GameOverlay.h/.cpp   # In-game overlays
│   │
│   └── utils/                 # Utilities
│       ├── GameConstants.h    # Game constants
│       ├── GameConfig.h       # Configuration structures
│       ├── SimpleJson.h       # Simple JSON parser
│       └── VectorMath.h       # Vector math utilities
│
└── tests/                     # Test code
    ├── TestFramework.h        # Test infrastructure
    ├── test_physics.cpp       # Physics validation tests
    └── test_ui.cpp            # UI component tests
```

## Module Organization

### Core Systems (`src/core/`)
Game loop, screen management, settings, and foundational systems.

### Game Logic (`src/game/`)
Gameplay mechanics, physics, entities, and controllers.

### Networking (`src/network/`)
Client-server networking, protocols, and matchmaking integration.

### Screens (`src/screens/`)
Different game screens (gameplay, menus, overlays).

### UI (`src/ui/`)
Reusable UI components and widgets.

### Utils (`src/utils/`)
Helper functions, constants, and utilities.

## Key Files

- **CMakeLists.txt** - Build configuration with SFML and ENet dependencies
- **config.json** - User configuration (lefty mode, colors, online settings)
- **scripts/build.sh** - Main build script with dependency checking
- **docs/ARCHITECTURE.md** - Detailed system architecture documentation
- **coordinator/** - Standalone Go service for matchmaking

## Build Outputs

All build artifacts go into `build/`:
- `sumo_balls` - Game client
- `sumo_balls_server` - Dedicated server
- `test_physics` - Physics validation tests
- `test_ui` - UI component tests

## Configuration Files

- `config.json` - Main user configuration (at root for easy access)
- `config/*.json` - Test configurations for multi-client testing

## Logs

All log files go into `logs/` directory (gitignored).

## Scripts

All utility scripts are in `scripts/` directory and should be run from project root:
```bash
./scripts/build.sh          # Build project
./scripts/run-server.sh     # Start server
./scripts/run-client.sh     # Start client
./scripts/test_online.sh    # Test online functionality
```

## Documentation

All documentation is in `docs/` directory:
- Architecture and design docs
- Integration guides
- Deployment instructions
- Development notes

## Tests

Test code is in `tests/` directory with shared test framework.
Test executables are built to `build/`.
