#include "GameScreen.h"
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
    } catch(const std::exception& e) {
        throw;
    } catch(...) {
        throw;
    }
}

void GameScreen::resolvePlayerCollisions() {
    const float COLLISION_ELASTICITY = 0.8f;  // Energy retention on collision
    
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
                float pushDistance = overlap / 2.0f + 0.1f;  // Small extra to prevent overlap
                
                // Normalized direction
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Push players apart
                players[i].move(sf::Vector2f(-nx * pushDistance, -ny * pushDistance));
                players[j].move(sf::Vector2f(nx * pushDistance, ny * pushDistance));
                
                // Exchange velocity components along collision normal (simplified impulse response)
                sf::Vector2f vel1 = players[i].getVelocity();
                sf::Vector2f vel2 = players[j].getVelocity();
                
                float vel1Normal = vel1.x * nx + vel1.y * ny;
                float vel2Normal = vel2.x * nx + vel2.y * ny;
                
                // Only apply if moving toward each other
                if(vel1Normal > vel2Normal) {
                    float avgVel = (vel1Normal + vel2Normal) * 0.5f * COLLISION_ELASTICITY;
                    
                    sf::Vector2f impulse1(-(vel1Normal - avgVel) * nx, -(vel1Normal - avgVel) * ny);
                    sf::Vector2f impulse2((vel2Normal - avgVel) * nx, (vel2Normal - avgVel) * ny);
                    
                    players[i].addVelocity(impulse1);
                    players[j].addVelocity(impulse2);
                }
            }
        }
    }
}
