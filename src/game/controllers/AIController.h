// AIController.h
#pragma once
#include "Controller.h"
#include <random>
#include <utility>

enum class DifficultyLevel { VeryEasy, Easy, Medium, Hard, Veteran };

struct DifficultyProfile {
    float reactionLag;       // seconds between decisions
    float aggression;        // attack weight
    float edgeFear;          // center bias near edge
    float shrinkFear;        // pre-emptive shrink bias
    float crowdAvoid;        // sidestep when crowded
    float attackBias;        // outward push component
    float burstChance;       // probability per second to start a burst
    float burstDuration;     // seconds the burst lasts
    float jitter;            // radians/sec wander jitter
    float wanderStrength;    // wander mix scale
};

class AIController : public Controller {
public:
    AIController(DifficultyLevel level = DifficultyLevel::Medium);
    AIController(float difficultyScalar); // compatibility: 0..1 maps to profiles

    Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const Vec2& selfVelocity,
        const std::vector<std::pair<Vec2, Vec2>>& otherPlayers,
        const Vec2& arenaCenter,
        float currentArenaRadius,
        float arenaAge
    ) override;

private:
    DifficultyLevel level;
    DifficultyProfile profile;
    float decisionTimer = 0.f;
    float reactionAccumulator = 0.f;
    Vec2 cachedDirection{0.f, 0.f};
    Vec2 lastDirection{0.f, 0.f};
    int patrolSign = 1;          // tangential direction (+1/-1)
    float edgeCooldown = 0.f;    // prevent oscillation flips
    bool burstActive = false;
    float burstTimer = 0.f;
    float burstCooldown = 0.f;   // time until next burst can start
    float wanderAngle = 0.f;

    static DifficultyProfile makeProfile(DifficultyLevel lvl);

    // Random number generator
    mutable std::mt19937 rng{std::random_device{}()};
};