#include "GamePhysics.h"
#include "PhysicsValidator.h"
#include <cmath>
#include <algorithm>

void GamePhysics::createExplosion(sf::Vector2f position, sf::Vector2f velocity) {
    // Validate inputs
    PhysicsValidator::assertPositionValid(position, "createExplosion");
    
    const int PARTICLE_COUNT = 18;
    const float PARTICLE_LIFETIME = 0.6f;
    const float PARTICLE_RADIUS = 4.f;
    const float VELOCITY_MULTIPLIER = 1.5f;
    
    // Get magnitude of player velocity to use as base for particle spread
    float velocityMagnitude = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if(velocityMagnitude < 10.f) {
        velocityMagnitude = 50.f;  // Minimum spread if player was barely moving
    }
    
    // Create particles in radial pattern
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        // Angle for this particle (0 to 360 degrees, spread evenly)
        float angle = (360.f / PARTICLE_COUNT) * i;
        float radians = angle * 3.14159265f / 180.f;
        
        // Base direction from angle
        float dirX = std::cos(radians);
        float dirY = std::sin(radians);
        
        // Add player's velocity direction component for more dynamic effect
        float velocityX = velocity.x + (dirX * velocityMagnitude * VELOCITY_MULTIPLIER);
        float velocityY = velocity.y + (dirY * velocityMagnitude * VELOCITY_MULTIPLIER);
        
        particles.emplace_back(Particle{
            position,
            sf::Vector2f(velocityX, velocityY),
            PARTICLE_LIFETIME,
            PARTICLE_LIFETIME,
            PARTICLE_RADIUS
        });
    }
}

void GamePhysics::updateParticles(float dt) {
    for(auto it = particles.begin(); it != particles.end(); ) {
        it->timeRemaining -= dt;
        
        if(it->timeRemaining <= 0.f) {
            it = particles.erase(it);
        } else {
            // Update position
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            
            // Validate particle position
            PhysicsValidator::validateAndClampPosition(it->position);
            
            // Apply slight gravity/drag
            it->velocity.y += 50.f * dt;  // Gravity
            it->velocity.x *= 0.98f;      // Drag
            it->velocity.y *= 0.98f;
            
            ++it;
        }
    }
}

void GamePhysics::resolvePlayerCollisions(std::vector<PlayerEntity>& players) {
    const float RESTITUTION = 2.05f;  // Higher elasticity for stronger rebounds
    
    for(size_t i = 0; i < players.size(); i++) {
        for(size_t j = i + 1; j < players.size(); j++) {
            if(!players[i].isAlive() || !players[j].isAlive()) continue;
            
            sf::Vector2f pos1 = players[i].getPosition();
            sf::Vector2f pos2 = players[j].getPosition();
            
            // Validate positions before collision detection
            PhysicsValidator::assertPositionValid(pos1, "player collision");
            PhysicsValidator::assertPositionValid(pos2, "player collision");
            
            float radius1 = players[i].getRadius();
            float radius2 = players[j].getRadius();
            float minDistance = radius1 + radius2;
            
            // Use squared distance for faster collision check (avoid sqrt)
            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float distSq = dx * dx + dy * dy;
            float minDistSq = minDistance * minDistance;
            
            if(distSq < minDistSq && distSq > 0.000001f) {
                // Collision detected - now calculate actual distance for resolution
                float distance = std::sqrt(distSq);
                float overlap = minDistance - distance;
                float pushDistance = overlap * 0.5f + 3.0f;  // Extra separation force
                
                // Normalized collision normal
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Separate overlapping balls with more force
                players[i].move(sf::Vector2f(-nx * pushDistance, -ny * pushDistance));
                players[j].move(sf::Vector2f(nx * pushDistance, ny * pushDistance));
                
                // Get masses and velocities
                float mass1 = players[i].getMass();
                float mass2 = players[j].getMass();
                sf::Vector2f vel1 = players[i].getVelocity();
                sf::Vector2f vel2 = players[j].getVelocity();
                
                // Calculate velocities along collision normal
                float vel1Normal = vel1.x * nx + vel1.y * ny;
                float vel2Normal = vel2.x * nx + vel2.y * ny;
                
                // Calculate velocities perpendicular to collision normal (unchanged)
                float vel1Tangent = vel1.x * (-ny) + vel1.y * nx;
                float vel2Tangent = vel2.x * (-ny) + vel2.y * nx;
                
                // Only resolve if objects are moving toward each other
                if(vel1Normal - vel2Normal <= 0) continue;
                
                // Conservation of momentum + restitution
                // Formula: v'n = ((m1*v1n + m2*v2n) ± restitution*m2*(v2n-v1n)) / (m1 + m2)
                float totalMass = mass1 + mass2;
                
                float newVel1Normal = ((mass1 * vel1Normal + mass2 * vel2Normal) + 
                                      mass2 * RESTITUTION * (vel2Normal - vel1Normal)) / totalMass;
                float newVel2Normal = ((mass1 * vel1Normal + mass2 * vel2Normal) + 
                                      mass1 * RESTITUTION * (vel1Normal - vel2Normal)) / totalMass;
                
                // Reconstruct velocities from normal and tangent components
                sf::Vector2f newVel1(
                    newVel1Normal * nx + vel1Tangent * (-ny),
                    newVel1Normal * ny + vel1Tangent * nx
                );
                sf::Vector2f newVel2(
                    newVel2Normal * nx + vel2Tangent * (-ny),
                    newVel2Normal * ny + vel2Tangent * nx
                );
                
                // Apply boosted impulses for more dramatic knockback
                const float impulseBoost = 1.15f;
                sf::Vector2f impulse1 = (newVel1 - vel1) * impulseBoost;
                sf::Vector2f impulse2 = (newVel2 - vel2) * impulseBoost;
                
                // Validate impulses before applying
                PhysicsValidator::assertVelocityValid(impulse1, "collision impulse");
                PhysicsValidator::assertVelocityValid(impulse2, "collision impulse");

                players[i].addVelocity(impulse1);
                players[j].addVelocity(impulse2);
            }
        }
    }
}
