// AIController.cpp
#include "AIController.h"
#include <cmath>
#include <random>

static sf::Vector2f normalize(const sf::Vector2f& v){
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0.f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

static float magnitude(const sf::Vector2f& v){
    return std::sqrt(v.x * v.x + v.y * v.y);
}

AIController::AIController(float diff) : difficulty(diff) {}

sf::Vector2f AIController::getMovementDirection(
    float dt,
    const sf::Vector2f& selfPosition,
    const std::vector<sf::Vector2f>& otherPlayers,
    const sf::Vector2f& arenaCenter,
    float arenaRadius
) {
    // Update decision timer (recalculate every 0.3-0.5 seconds)
    decisionTimer -= dt;
    if(decisionTimer <= 0.f) {
        decisionTimer = 0.3f + (0.2f * (1.f - difficulty));  // More frequent decisions on hard
        
        sf::Vector2f targetDirection{0.f, 0.f};
        
        if(!otherPlayers.empty()) {
            // Focus on closest opponent
            sf::Vector2f opponent = otherPlayers[0];
            sf::Vector2f toOpponent = opponent - selfPosition;
            float distToOpponent = magnitude(toOpponent);
            
            if(distToOpponent < 100.f) {
                // Too close - back away
                targetDirection = -normalize(toOpponent);
            } else if(distToOpponent < 300.f) {
                // Medium distance - charge toward them
                targetDirection = normalize(toOpponent);
            } else {
                // Far away - move toward them
                targetDirection = normalize(toOpponent);
            }
        }
        
        // Safety: stay away from arena edges
        sf::Vector2f toCenter = arenaCenter - selfPosition;
        float distToCenter = magnitude(toCenter);
        float safeDistance = arenaRadius - 50.f;  // Stay 50 pixels from edge
        
        if(distToCenter > safeDistance) {
            // Moving away from center - push back toward it
            sf::Vector2f correction = normalize(toCenter) * (1.f - difficulty);
            targetDirection = normalize(targetDirection + correction);
        }
        
        // Add random jitter for unpredictability (more on lower difficulty)
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        
        float jitterAmount = (1.f - difficulty) * 0.3f;
        targetDirection.x += dist(rng) * jitterAmount;
        targetDirection.y += dist(rng) * jitterAmount;
        
        cachedDirection = normalize(targetDirection);
    }
    
    return cachedDirection;
}