// AIController.h
#pragma once
#include "Controller.h"
#include <random>

class AIController : public Controller {
public:
    AIController(float difficulty = 0.5f);  // 0.0 = easy, 1.0 = hard
    
    sf::Vector2f getMovementDirection(
        float dt,
        const sf::Vector2f& selfPosition,
        const std::vector<sf::Vector2f>& otherPlayers,
        const sf::Vector2f& arenaCenter,
        float arenaRadius
    ) override;

private:
    float difficulty;
    float decisionTimer = 0.f;
    sf::Vector2f cachedDirection{0.f, 0.f};
    sf::Vector2f lastDirection{0.f, 0.f};
    float aggressiveness;  // Personality trait: how aggressive this AI is
    float caution;         // Personality trait: how cautious near edges
    int patrolSign = 1;    // Persistent tangential movement direction (+1/-1)
    float edgeCooldown = 0.f; // Prevent rapid direction flips near the boundary
    // Aggression bursts: occasional 5-second windows of higher aggression
    bool burstActive = false;
    float burstTimer = 0.f;       // counts down during burst
    float nextBurstDelay = 0.f;   // time until next burst starts
    // Wander/curvature to avoid straight-line traversals
    float wanderAngle = 0.f;      // evolving heading noise
    float wanderJitter = 2.4f;    // radians/sec noise amplitude
    float wanderStrength = 0.32f; // scale of wander vector blended into steering
    
    // Random number generator (centralized, reused across frames)
    mutable std::mt19937 rng{std::random_device{}()};

    // Helper methods
    float calculateAdvantage(float selfDist, float opponentDist) const;
    int selectTarget(const sf::Vector2f& selfPosition, 
                     const std::vector<sf::Vector2f>& opponents,
                     const sf::Vector2f& arenaCenter) const;
};