// AIController.h
#pragma once
#include "Controller.h"
#include <random>

class AIController : public Controller {
public:
    AIController(float difficulty = 0.5f);  // 0.0 = easy, 1.0 = hard
    
    Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const std::vector<Vec2>& otherPlayers,
        const Vec2& arenaCenter,
        float arenaRadius
    ) override;

private:
    float difficulty;
    float decisionTimer = 0.f;
    Vec2 cachedDirection{0.f, 0.f};
    Vec2 lastDirection{0.f, 0.f};
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
};