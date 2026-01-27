# Test Suite for Sumo Balls

Comprehensive test organization for the Sumo Balls game. Tests are organized by subsystem and test type.

## Test Organization

```
tests/
├── README.md                    # This file
├── TestFramework.h              # Custom test framework
├── TestRunner.cpp               # Main test runner
├── fixtures/                    # Shared test data and fixtures
├── unit/                        # Unit tests by subsystem
│   ├── core/                    # Logger, Settings tests
│   ├── game/                    # Physics, Simulation tests
│   ├── network/                 # Network protocol tests
│   └── ui/                      # UI component tests
├── integration/                 # Cross-component tests (planned)
└── e2e/                         # End-to-end tests
    └── smoke_test.cpp           # Basic smoke tests
```

## Running Tests

Build and run all tests:
```bash
# From project root
cd /home/soyal/sumo-balls
cmake --build build
./build/sumo_balls_test
```

Or use CTest:
```bash
cd build
ctest --output-on-failure
```

## Test Framework

We use a lightweight custom test framework that doesn't require external dependencies.

### Writing Tests

```cpp
#include "TestFramework.h"
#include "../../src/your/module.h"

bool testMyFeature(std::string& errorMsg) {
    // Arrange
    int expected = 42;
    
    // Act
    int actual = yourFunction();
    
    // Assert
    TEST_EQUAL(actual, expected, "Values should match");
    TEST_TRUE(actual > 0);
    TEST_FALSE(actual < 0);
    
    return true;  // Test passes
}

// Auto-register test
namespace {
    struct MyTestsRegistration {
        MyTestsRegistration() {
            test::TestSuite::instance().registerTest("Category::FeatureName", testMyFeature);
        }
    } myTests;
}
```

### Test Macros

- `TEST_ASSERT(condition, message)` - Assert a condition is true
- `TEST_EQUAL(expected, actual, message)` - Assert two values are equal
- `TEST_TRUE(condition)` - Assert condition is true
- `TEST_FALSE(condition)` - Assert condition is false

## Test Coverage

### Unit Tests

**Core Module** (`tests/unit/core/`)
- ✅ Logger: All log levels, timestamps, filtering
- ✅ Settings: Static members, colors, modes

**Game Module** (`tests/unit/game/`)
- ✅ Physics: Finite checks, position/velocity validation, clamping

**Network Module** (`tests/unit/network/`)
- ✅ NetProtocol: Message parsing and error handling

**UI Module** (`tests/unit/ui/`)
- ✅ UI Components: Button, toggles, rendering

### Integration Tests (Planned)
- Client-server communication
- Network message handling  
- Game server state management
- Full game flow scenarios

### E2E Tests
- Smoke test: Basic startup and shutdown

## File Organization

When adding tests, follow this naming and placement pattern:

| Test Type | Location | Naming |
|-----------|----------|--------|
| Unit - Core | `tests/unit/core/` | `{Component}Test.cpp` |
| Unit - Game | `tests/unit/game/` | `{Component}Test.cpp` |
| Unit - Network | `tests/unit/network/` | `{Protocol}Test.cpp` |
| Unit - UI | `tests/unit/ui/` | `{Component}Test.cpp` |
| Integration | `tests/integration/` | `{feature}_integration_test.cpp` |
| E2E | `tests/e2e/` | `{scenario}_e2e_test.cpp` |

## Contributing

When adding new features:
1. Write tests first (TDD approach recommended)
2. Place test file in appropriate `tests/unit/{subsystem}/` directory
3. Use auto-registration pattern for test discovery
4. Ensure all existing tests still pass: `./build/sumo_balls_test`
5. Update this README if adding new subsystem
