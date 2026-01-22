#pragma once
#include "../game/entities/PlayerEntity.h"
#include "../game/entities/Arena.h"
#include <SFML/Graphics.hpp>
#include <vector>

/// Particle effect for explosions
struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;        // Total lifetime in seconds
    float timeRemaining;   // Time left before particle dies
    float radius;
};

/// Manages physics simulation and particle effects
/// Handles collision detection, resolution, and explosion particles
class GamePhysics {
public:
    GamePhysics() = default;
    ~GamePhysics() = default;
    
    /// Resolve collisions between all players
    /// Updates player velocities based on elastic collisions
    void resolvePlayerCollisions(std::vector<PlayerEntity>& players);
    
    /// Create explosion particle effect at position
    /// @param position: Center of explosion
    /// @param velocity: Initial velocity of particle spread
    void createExplosion(sf::Vector2f position, sf::Vector2f velocity);
    
    /// Update all particles, removing dead ones
    /// @param dt: Delta time in seconds
    void updateParticles(float dt);
    
    /// Get current particles for rendering
    const std::vector<Particle>& getParticles() const { return particles; }
    
    /// Clear all particles
    void clearParticles() { particles.clear(); }

private:
    std::vector<Particle> particles;
};
