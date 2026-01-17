#include "GameScreen.h"
#include "menus/GameOverMenu.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>

GameScreen::GameScreen():
    arena({600.f, 450.f}, 300.f){
    auto humanCtrl = std::make_unique<HumanController>();
    players.emplace_back(sf::Vector2f(400.f, 450.f), std::move(humanCtrl));
    // In GameScreen constructor:
    auto aiCtrl = std::make_unique<AIController>(0.7f);  // 70% difficulty
    players.emplace_back(sf::Vector2f(800.f, 450.f), std::move(aiCtrl));
}

void GameScreen::update(sf::Time dt, sf::RenderWindow&) {
    try {
        frameCount++;
        
        // Pre-allocate positions vector to avoid repeated allocations
        positions.clear();
        positions.reserve(players.size());
        for(auto& p : players){
            if(p.isAlive()){
                positions.push_back(p.getPosition());
            }
        }

        for(auto& p : players){
            p.update(dt.asSeconds(), positions, arena.center, arena.radius);
        }
        
        resolvePlayerCollisions();
        
        // Check arena boundaries - mark players as dead if 50% outside arena
        for(auto& p : players) {
            if(p.isAlive()) {
                sf::Vector2f playerPos = p.getPosition();
                float playerRadius = p.getRadius();
                
                // Distance from player center to arena center
                float dx = playerPos.x - arena.center.x;
                float dy = playerPos.y - arena.center.y;
                float distToCenter = std::sqrt(dx * dx + dy * dy);
                
                // Player is dead if their center is more than (arenaRadius + 50% of playerRadius) away
                float deathDistance = arena.radius + playerRadius * 0.5f;
                
                if(distToCenter > deathDistance) {
                    // Create explosion before marking dead
                    createExplosion(playerPos, p.getVelocity());
                    p.setAlive(false);
                }
            }
        }
        
        // Update particles
        updateParticles(dt.asSeconds());
        
        // Check if game is over (only one player alive)
        if(!gameOver){
            int aliveCount = 0;
            for(auto& p : players){
                if(p.isAlive()) aliveCount++;
            }
            
            if(aliveCount <= 1) {
                gameOver = true;
                gameOverTime = 0.f;  // Start game over timer
            }
        } else {
            gameOverTime += dt.asSeconds();  // Track time since game ended
        }
    } catch(const std::exception& e) {
        throw;
    } catch(...) {
        throw;
    }
}

void GameScreen::render(sf::RenderWindow& window) {
    try {
        arena.render(window);

        for (auto& playerEntity : players) {
            if (playerEntity.isAlive()) {
                playerEntity.render(window);
            }
        }
        
        // Render particles
        for(auto& particle : particles) {
            sf::CircleShape particleShape(particle.radius);
            
            // Calculate alpha fade (fade out as particle dies)
            float alphaRatio = particle.timeRemaining / particle.lifetime;
            int alpha = static_cast<int>(255.f * alphaRatio);
            
            particleShape.setFillColor(sf::Color(255, 200, 100, alpha));  // Orange/yellow particles
            particleShape.setPosition(particle.position.x - particle.radius, 
                                     particle.position.y - particle.radius);
            
            window.draw(particleShape);
        }
        
        // If game is over, show overlay message
        if(gameOver) {
            // Draw semi-transparent overlay
            sf::RectangleShape overlay({1200.f, 900.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 100));
            window.draw(overlay);
            
            // Note: Menu will be handled by ScreenStack when we return to main menu
        }
    } catch(const std::exception& e) {
        throw;
    } catch(...) {
        throw;
    }
}

MenuAction GameScreen::getMenuAction() const {
    if(gameOver && gameOverTime >= 0.7f) {
        return MenuAction::MAIN_MENU;  // Return to main menu after particles finish fading
    }
    return MenuAction::NONE;
}

void GameScreen::resetMenuAction() {
    gameOverAction = MenuAction::NONE;
}

void GameScreen::createExplosion(sf::Vector2f position, sf::Vector2f velocity) {
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

void GameScreen::updateParticles(float dt) {
    for(auto it = particles.begin(); it != particles.end(); ) {
        it->timeRemaining -= dt;
        
        if(it->timeRemaining <= 0.f) {
            it = particles.erase(it);
        } else {
            // Update position
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            
            // Apply slight gravity/drag
            it->velocity.y += 50.f * dt;  // Gravity
            it->velocity.x *= 0.98f;      // Drag
            it->velocity.y *= 0.98f;
            
            ++it;
        }
    }
}

void GameScreen::resolvePlayerCollisions() {
    const float COLLISION_ELASTICITY = 1.5f;  // Energy amplification for bouncy collisions
    const float KNOCKBACK_MULTIPLIER = 1.2f;  // Additional boost for impact feel
    
    for(size_t i = 0; i < players.size(); i++) {
        for(size_t j = i + 1; j < players.size(); j++) {
            if(!players[i].isAlive() || !players[j].isAlive()) continue;
            
            sf::Vector2f pos1 = players[i].getPosition();
            sf::Vector2f pos2 = players[j].getPosition();
            
            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            float radius1 = players[i].getRadius();
            float radius2 = players[j].getRadius();
            float minDistance = radius1 + radius2;
            
            if(distance < minDistance && distance > 0.001f) {
                // Collision detected - push players apart
                float overlap = minDistance - distance;
                float pushDistance = overlap / 2.0f + 0.5f;  // Increased separation force
                
                // Normalized direction
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Push players apart with stronger force
                players[i].move(sf::Vector2f(-nx * pushDistance, -ny * pushDistance));
                players[j].move(sf::Vector2f(nx * pushDistance, ny * pushDistance));
                
                // Calculate relative velocity for more realistic collision response
                sf::Vector2f vel1 = players[i].getVelocity();
                sf::Vector2f vel2 = players[j].getVelocity();
                
                float vel1Normal = vel1.x * nx + vel1.y * ny;
                float vel2Normal = vel2.x * nx + vel2.y * ny;
                
                // Apply bounceback only if moving toward each other
                if(vel1Normal > vel2Normal) {
                    // Calculate restitution - how much velocity is transferred
                    float relativeVelocity = vel1Normal - vel2Normal;
                    float restitution = COLLISION_ELASTICITY * relativeVelocity * KNOCKBACK_MULTIPLIER;
                    
                    // Apply impulses (stronger for more noticeable bounceback)
                    sf::Vector2f impulse1(-restitution * nx, -restitution * ny);
                    sf::Vector2f impulse2(restitution * nx, restitution * ny);
                    
                    players[i].addVelocity(impulse1);
                    players[j].addVelocity(impulse2);
                }
            }
        }
    }
}
