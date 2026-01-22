#include "TestFramework.h"

// Forward declarations - tests are in other files
extern int main(int argc, char* argv[]);

// Note: Individual test files will auto-register their tests
// via the TEST() macro which uses static constructors

int main(int argc, char* argv[]) {
    return test::TestSuite::instance().runAll();
}
