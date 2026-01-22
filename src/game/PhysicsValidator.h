#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>
#include <cassert>

/// Physics validation utilities
/// Detects invalid state (NaN, Inf, out-of-bounds) to prevent silent corruption
namespace PhysicsValidator {
    
/// Maximum reasonable world coordinates
/// Values beyond this indicate a bug in physics simulation
constexpr float MAX_WORLD_COORD = 10000.f;
constexpr float MIN_WORLD_COORD = -10000.f;

/// Maximum reasonable velocity (pixels per second)
/// Values beyond this indicate unstable physics
constexpr float MAX_VELOCITY = 5000.f;

/// Check if a value is finite (not NaN or Inf)
inline bool isFinite(float value) {
    return std::isfinite(value);
}

/// Check if a position is within reasonable bounds
inline bool isPositionValid(const sf::Vector2f& pos) {
    return isFinite(pos.x) && isFinite(pos.y) &&
           pos.x > MIN_WORLD_COORD && pos.x < MAX_WORLD_COORD &&
           pos.y > MIN_WORLD_COORD && pos.y < MAX_WORLD_COORD;
}

/// Check if a velocity is within reasonable bounds
inline bool isVelocityValid(const sf::Vector2f& vel) {
    if (!isFinite(vel.x) || !isFinite(vel.y)) return false;
    float magnitude = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    return magnitude < MAX_VELOCITY;
}

/// Assert position is valid (debug builds only)
inline void assertPositionValid(const sf::Vector2f& pos, const char* context = "") {
    if (!isPositionValid(pos)) {
        std::cerr << "[Physics Error] Invalid position at " << context 
                  << ": (" << pos.x << ", " << pos.y << ")" << std::endl;
        assert(isPositionValid(pos) && "Position contains NaN/Inf or out of bounds");
    }
}

/// Assert velocity is valid (debug builds only)
inline void assertVelocityValid(const sf::Vector2f& vel, const char* context = "") {
    if (!isVelocityValid(vel)) {
        std::cerr << "[Physics Error] Invalid velocity at " << context
                  << ": (" << vel.x << ", " << vel.y << ")" << std::endl;
        assert(isVelocityValid(vel) && "Velocity contains NaN/Inf or too large");
    }
}

/// Validate and clamp position to safe bounds
/// Returns true if clamping was needed
inline bool validateAndClampPosition(sf::Vector2f& pos) {
    bool clamped = false;
    
    // Check for NaN/Inf - replace with origin
    if (!isFinite(pos.x) || !isFinite(pos.y)) {
        std::cerr << "[Physics Warning] NaN/Inf position detected, resetting to origin" << std::endl;
        pos = sf::Vector2f(600.f, 450.f);  // Game center
        return true;
    }
    
    // Clamp to bounds
    if (pos.x < MIN_WORLD_COORD) { pos.x = MIN_WORLD_COORD; clamped = true; }
    if (pos.x > MAX_WORLD_COORD) { pos.x = MAX_WORLD_COORD; clamped = true; }
    if (pos.y < MIN_WORLD_COORD) { pos.y = MIN_WORLD_COORD; clamped = true; }
    if (pos.y > MAX_WORLD_COORD) { pos.y = MAX_WORLD_COORD; clamped = true; }
    
    return clamped;
}

/// Validate and clamp velocity to safe bounds
/// Returns true if clamping was needed
inline bool validateAndClampVelocity(sf::Vector2f& vel) {
    bool clamped = false;
    
    // Check for NaN/Inf - reset to zero
    if (!isFinite(vel.x) || !isFinite(vel.y)) {
        std::cerr << "[Physics Warning] NaN/Inf velocity detected, resetting to zero" << std::endl;
        vel = sf::Vector2f(0.f, 0.f);
        return true;
    }
    
    // Clamp magnitude
    float magnitude = std::sqrt(vel.x * vel.x + vel.y * vel.y);
    if (magnitude > MAX_VELOCITY) {
        float scale = MAX_VELOCITY / magnitude;
        vel.x *= scale;
        vel.y *= scale;
        clamped = true;
    }
    
    return clamped;
}

} // namespace PhysicsValidator
