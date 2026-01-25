#include "TestFramework.h"

// All tests are auto-registered via the TEST() macro
// This executable simply runs all registered tests

int main() {
    return test::TestSuite::instance().runAll();
}
