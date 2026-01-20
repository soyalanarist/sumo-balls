#include "GameScreen.h"
#include "menus/GameOverMenu.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../core/Settings.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>

GameScreen::GameScreen():
    arena({600.f, 450.f}, 300.f),
    initialArenaRadius(300.f){
    // Human player at bottom-left with selected color
    auto humanCtrl = std::make_unique<HumanController>();
    players.emplace_back(sf::Vector2f(400.f, 550.f), std::move(humanCtrl), Settings::getPlayerColor());
    
    // 5 AI players positioned around the arena with varied difficulties
    // Positions arranged in a circle around arena center
    auto ai1 = std::make_unique<AIController>(0.5f);  // 50% difficulty
    players.emplace_back(sf::Vector2f(800.f, 550.f), std::move(ai1));
    
    auto ai2 = std::make_unique<AIController>(0.6f);  // 60% difficulty
    players.emplace_back(sf::Vector2f(750.f, 350.f), std::move(ai2));
    
    auto ai3 = std::make_unique<AIController>(0.7f);  // 70% difficulty
    players.emplace_back(sf::Vector2f(600.f, 250.f), std::move(ai3));
    
    auto ai4 = std::make_unique<AIController>(0.65f);  // 65% difficulty
    players.emplace_back(sf::Vector2f(450.f, 350.f), std::move(ai4));
    
    auto ai5 = std::make_unique<AIController>(0.55f);  // 55% difficulty
    players.emplace_back(sf::Vector2f(600.f, 500.f), std::move(ai5));
}

void GameScreen::update(sf::Time dt, [[maybe_unused]] sf::RenderWindow& window) {
    try{
        frameCount++;
        
        // Handle countdown before game starts
        if(countdownActive) {
            countdownTime -= dt.asSeconds();
            if(countdownTime <= 0.f) {
                countdownActive = false;
                countdownTime = 0.f;
            }
            // During countdown, don't update game logic
            return;
        }
        
        // Increment game time only if game is not over
        if(!gameOver) {
            gameTime += dt.asSeconds();
        }
        
        // Check for pause key (P)
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            menuAction = MenuAction::PAUSE;
            return;  // Don't update game state if pausing
        }
        
        menuAction = MenuAction::NONE;  // Clear any pending actions
        positions.clear();
        positions.reserve(players.size());
        for(auto& p : players){
            if(p.isAlive()){
                positions.push_back(p.getPosition());
            }
        }

        for(auto& p : players){
            float speedMult = getSpeedMultiplier();
            p.update(dt.asSeconds(), positions, arena.center, arena.radius, speedMult);
        }
        
        resolvePlayerCollisions();
        
        // Shrink arena over time (continuous shrinking until game ends)
        if(!gameOver) {
            float shrinkRate = 5.0f;  // pixels per second
            float newRadius = 300.f - (gameTime * shrinkRate);
            if(newRadius > 0.f) {
                arena.setRadius(newRadius);
            }
        }
        
        // Check arena boundaries - mark players as dead if 50% outside arena
        for(auto& p : players){
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

float GameScreen::getSpeedMultiplier() const {
    // Speed multiplier increases as arena shrinks
    // At full size: 1.0x, at half size: 1.5x, at 1/4 size: 2.0x
    float currentRadius = arena.radius;
    if(initialArenaRadius <= 0.f) return 1.0f;
    float shrinkRatio = currentRadius / initialArenaRadius;
    // Linear scaling: 1.0 + (1 - shrinkRatio)
    return 1.0f + (1.0f - shrinkRatio);
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
        
        // Countdown overlay before game starts
        if(countdownActive) {
            // Draw greyed out overlay
            sf::RectangleShape overlay({1200.f, 900.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            
            // Load font for countdown numbers
            static sf::Font font;
            static bool fontLoaded = false;
            if(!fontLoaded) {
                if(font.loadFromFile("assets/arial.ttf")) {
                    fontLoaded = true;
                }
            }
            
            if(fontLoaded) {
                // Determine countdown number (3, 2, 1)
                int countdownNumber = static_cast<int>(std::ceil(countdownTime));
                if(countdownNumber > 0 && countdownNumber <= 3) {
                    sf::Text countdownText;
                    countdownText.setFont(font);
                    countdownText.setString(std::to_string(countdownNumber));
                    countdownText.setCharacterSize(200);
                    
                    // Use player's color
                    sf::Color playerColor = Settings::getPlayerColor();
                    
                    // Calculate fade effect (fade in and out within each second)
                    float timeInSecond = countdownTime - std::floor(countdownTime);
                    float fadeAlpha = 1.0f;
                    
                    // Fade in during first 0.2s, fade out during last 0.3s
                    if(timeInSecond > 0.7f) {
                        fadeAlpha = (1.0f - timeInSecond) / 0.3f;  // Fade out
                    } else if(timeInSecond < 0.2f) {
                        fadeAlpha = timeInSecond / 0.2f;  // Fade in
                    }
                    
                    playerColor.a = static_cast<sf::Uint8>(255 * fadeAlpha);
                    countdownText.setFillColor(playerColor);
                    
                    // Center the text
                    sf::FloatRect textBounds = countdownText.getLocalBounds();
                    countdownText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                           textBounds.top + textBounds.height / 2.0f);
                    countdownText.setPosition(600.f, 400.f);
                    
                    window.draw(countdownText);
                }
            }
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
    // Return pause action if requested
    if(menuAction == MenuAction::PAUSE) {
        return MenuAction::PAUSE;
    }
    
    // Return main menu action after game is over and particles finished
    if(gameOver && gameOverTime >= 0.7f) {
        return MenuAction::MAIN_MENU;
    }
    
    return MenuAction::NONE;
}

void GameScreen::resetMenuAction() {
    menuAction = MenuAction::NONE;
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
    const float RESTITUTION = 1.8f;  // Elasticity coefficient - even more dramatic bounces!
    
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
                // Collision detected
                float overlap = minDistance - distance;
                float pushDistance = overlap / 2.0f + 1.5f;  // Increased separation force
                
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
                if(vel1Normal - vel2Normal <= 0) return;
                
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
                
                // Apply new velocities (set absolute velocity, not impulse)
                sf::Vector2f impulse1 = newVel1 - vel1;
                sf::Vector2f impulse2 = newVel2 - vel2;
                
                players[i].addVelocity(impulse1);
                players[j].addVelocity(impulse2);
            }
        }
    }
}

