#include "TestFramework.h"
#include "../src/game/PhysicsValidator.h"
#include <SFML/System/Vector2.hpp>

bool testFiniteCheck(std::string& msg) {
    if (!PhysicsValidator::isFinite(0.f)) {
        msg = "0 should be finite";
        return false;
    }
    if (!PhysicsValidator::isFinite(100.f)) {
        msg = "100 should be finite";
        return false;
    }
    if (!PhysicsValidator::isFinite(-50.f)) {
        msg = "-50 should be finite";
        return false;
    }
    return true;
}

bool testPositionValidation(std::string& msg) {
    sf::Vector2f validPos(600.f, 450.f);
    if (!PhysicsValidator::isPositionValid(validPos)) {
        msg = "Normal position should be valid";
        return false;
    }
    
    sf::Vector2f edgePos(9999.f, -9999.f);
    if (!PhysicsValidator::isPositionValid(edgePos)) {
        msg = "Edge position should be valid";
        return false;
    }
    
    sf::Vector2f outOfBounds(20000.f, 0.f);
    if (PhysicsValidator::isPositionValid(outOfBounds)) {
        msg = "Out of bounds position should be invalid";
        return false;
    }
    return true;
}

bool testVelocityValidation(std::string& msg) {
    sf::Vector2f normalVel(100.f, 50.f);
    if (!PhysicsValidator::isVelocityValid(normalVel)) {
        msg = "Normal velocity should be valid";
        return false;
    }
    
    sf::Vector2f zeroVel(0.f, 0.f);
    if (!PhysicsValidator::isVelocityValid(zeroVel)) {
        msg = "Zero velocity should be valid";
        return false;
    }
    
    sf::Vector2f extremeVel(10000.f, 10000.f);
    if (PhysicsValidator::isVelocityValid(extremeVel)) {
        msg = "Extreme velocity should be invalid";
        return false;
    }
    return true;
}

bool testPositionClamping(std::string& msg) {
    sf::Vector2f pos(20000.f, -20000.f);
    bool clamped = PhysicsValidator::validateAndClampPosition(pos);
    
    if (!clamped) {
        msg = "Out of bounds position should be clamped";
        return false;
    }
    
    // After clamping, position should be at bounds (which are NOT considered valid by strict check)
    // The validator uses strict inequality (> MIN, < MAX) so boundary values are invalid
    // But clamping puts values AT the boundary, so this is expected behavior
    if (pos.x != PhysicsValidator::MAX_WORLD_COORD) {
        msg = "X should be clamped to max";
        return false;
    }
    if (pos.y != PhysicsValidator::MIN_WORLD_COORD) {
        msg = "Y should be clamped to min";
        return false;
    }
    return true;
}

bool testVelocityClamping(std::string& msg) {
    sf::Vector2f vel(10000.f, 10000.f);
    bool clamped = PhysicsValidator::validateAndClampVelocity(vel);
    
    if (!clamped) {
        msg = "Extreme velocity should be clamped";
        return false;
    }
    
    // After clamping, magnitude should be exactly MAX_VELOCITY
    float magnitude = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (std::abs(magnitude - PhysicsValidator::MAX_VELOCITY) > 0.1f) {
        msg = "Clamped velocity magnitude should be exactly MAX_VELOCITY";
        return false;
    }
    return true;
}

int main() {
    test::TestSuite::instance().registerTest("Finite value check", testFiniteCheck);
    test::TestSuite::instance().registerTest("Position validation", testPositionValidation);
    test::TestSuite::instance().registerTest("Velocity validation", testVelocityValidation);
    test::TestSuite::instance().registerTest("Position clamping", testPositionClamping);
    test::TestSuite::instance().registerTest("Velocity clamping", testVelocityClamping);
    
    int result = test::TestSuite::instance().runAll();
    return result;
}
