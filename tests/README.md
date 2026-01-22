# Test Suite for Sumo Balls

This directory contains the testing infrastructure for the Sumo Balls game.

## Test Organization

```
tests/
├── TestFramework.h         # Lightweight custom test framework
├── TestRunner.cpp          # Main test executable entry point
├── unit/                   # Unit tests for individual modules
│   ├── SettingsTest.cpp    # Tests for Settings module
│   ├── NetProtocolTest.cpp # Tests for network protocol
│   └── LoggerTest.cpp      # Tests for logging system
└── integration/            # Integration tests (planned)
```

## Running Tests

Build and run all tests:
```bash
cd build
cmake ..
cmake --build .
./sumo_balls_test
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
#include "../TestFramework.h"
#include "../../src/your/module.h"

TEST(ModuleName, TestName) {
    // Arrange
    int expected = 42;
    
    // Act
    int actual = yourFunction();
    
    // Assert
    TEST_EQUAL(expected, actual, "Values should match");
    TEST_TRUE(actual > 0);
    TEST_FALSE(actual < 0);
    TEST_ASSERT(actual == 42, "Custom condition message");
    
    return true;  // Test passes
}
```

### Test Macros

- `TEST(suite, name)` - Define a test function
- `TEST_ASSERT(condition, message)` - Assert a condition is true
- `TEST_EQUAL(expected, actual, message)` - Assert two values are equal
- `TEST_TRUE(condition)` - Assert condition is true
- `TEST_FALSE(condition)` - Assert condition is false

## Test Coverage

### Core Module

- ✅ Settings static member access
- ✅ Color management and validation
- ✅ Fullscreen and lefty mode toggles
- ✅ Configuration save/load

### Network Module

- ✅ ParseResult success/failure cases
- ✅ Error message formatting
- ✅ Error type validation
- ✅ Context preservation in errors

### Logging System

- ✅ All log levels (DEBUG, INFO, WARN, ERROR, CRITICAL)
- ✅ Timestamp generation
- ✅ Module tagging
- ✅ File output
- ✅ Log level filtering
- ✅ Multiple module logging

## Future Test Plans

### Unit Tests (Planned)
- Simulation physics tests
- Input system tests
- UI component tests
- Game entity tests (Player, Arena)

### Integration Tests (Planned)
- Client-server communication
- Matchmaking flow
- Full game session

### Performance Tests (Planned)
- Network serialization benchmarks
- Simulation tick performance
- Rendering frame rate

## Test Results

Last test run: All 23 tests passing ✅

```
Settings: 7 tests
NetProtocol: 6 tests
Logger: 10 tests
```

## Continuous Testing

For development, you can watch for changes and auto-run tests:

```bash
# Simple approach
while true; do inotifywait -r -e modify src/ tests/; cmake --build build && ./build/sumo_balls_test; done
```

## Contributing

When adding new features:
1. Write tests first (TDD approach recommended)
2. Ensure all existing tests still pass
3. Add integration tests for complex features
4. Update this README with new test coverage

## Test Philosophy

- **Unit tests**: Fast, isolated, test single components
- **Integration tests**: Test component interactions
- **Keep tests simple**: Each test should verify one thing
- **Descriptive names**: Test names should describe what they verify
- **Independent tests**: Tests shouldn't depend on each other
