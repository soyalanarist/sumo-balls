#include "TestFramework.h"
#include "../src/game/PhysicsValidator.h"
#include "../src/utils/VectorMath.h"
#include <cmath>

bool testPhysicsFiniteCheck(std::string& errorMsg) {
    TEST_TRUE(PhysicsValidator::isFinite(0.f));
    TEST_TRUE(PhysicsValidator::isFinite(100.f));
    TEST_TRUE(PhysicsValidator::isFinite(-50.f));
    return true;
}

bool testPhysicsPositionValidation(std::string& errorMsg) {
    Vec2 validPos(600.f, 450.f);
    TEST_TRUE(PhysicsValidator::isPositionValid(validPos));
    
    Vec2 edgePos(9999.f, -9999.f);
    TEST_TRUE(PhysicsValidator::isPositionValid(edgePos));
    
    Vec2 outOfBounds(20000.f, 0.f);
    TEST_FALSE(PhysicsValidator::isPositionValid(outOfBounds));
    return true;
}

bool testPhysicsVelocityValidation(std::string& errorMsg) {
    Vec2 normalVel(100.f, 50.f);
    TEST_TRUE(PhysicsValidator::isVelocityValid(normalVel));
    
    Vec2 zeroVel(0.f, 0.f);
    TEST_TRUE(PhysicsValidator::isVelocityValid(zeroVel));
    
    Vec2 extremeVel(10000.f, 10000.f);
    TEST_FALSE(PhysicsValidator::isVelocityValid(extremeVel));
    return true;
}

bool testPhysicsPositionClamping(std::string& errorMsg) {
    Vec2 pos(20000.f, -20000.f);
    bool clamped = PhysicsValidator::validateAndClampPosition(pos);
    
    TEST_TRUE(clamped);
    TEST_EQUAL(pos.x, PhysicsValidator::MAX_WORLD_COORD, "X should be clamped to max");
    TEST_EQUAL(pos.y, PhysicsValidator::MIN_WORLD_COORD, "Y should be clamped to min");
    return true;
}

bool testPhysicsVelocityClamping(std::string& errorMsg) {
    Vec2 vel(10000.f, 10000.f);
    bool clamped = PhysicsValidator::validateAndClampVelocity(vel);
    
    TEST_TRUE(clamped);
    float magnitude = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (std::abs(magnitude - PhysicsValidator::MAX_VELOCITY) > 0.1f) {
        errorMsg = "Clamped velocity magnitude should be exactly MAX_VELOCITY";
        return false;
    }
    return true;
}

// Auto-register tests
namespace {
    struct PhysicsTestsRegistration {
        PhysicsTestsRegistration() {
            test::TestSuite::instance().registerTest("Physics::FiniteCheck", testPhysicsFiniteCheck);
            test::TestSuite::instance().registerTest("Physics::PositionValidation", testPhysicsPositionValidation);
            test::TestSuite::instance().registerTest("Physics::VelocityValidation", testPhysicsVelocityValidation);
            test::TestSuite::instance().registerTest("Physics::PositionClamping", testPhysicsPositionClamping);
            test::TestSuite::instance().registerTest("Physics::VelocityClamping", testPhysicsVelocityClamping);
        }
    } physicsTests;
}
