#include "AIController.h"
#include "../../utils/VectorMath.h"
#include <algorithm>
#include <cmath>
#include <limits>

static inline float clampf(float v, float a, float b){ return std::max(a, std::min(v, b)); }

DifficultyProfile AIController::makeProfile(DifficultyLevel lvl) {
    switch (lvl) {
        case DifficultyLevel::VeryEasy:
            return {0.28f, 0.55f, 0.85f, 0.75f, 0.60f, 0.40f, 0.06f, 1.6f, 1.2f, 0.10f};
        case DifficultyLevel::Easy:
            return {0.22f, 0.95f, 0.60f, 0.62f, 0.42f, 0.55f, 0.08f, 2.0f, 1.5f, 0.09f};
        case DifficultyLevel::Medium:
            return {0.16f, 1.20f, 0.38f, 0.55f, 0.32f, 0.65f, 0.10f, 2.3f, 1.7f, 0.08f};
        case DifficultyLevel::Hard:
            return {0.10f, 1.35f, 0.30f, 0.48f, 0.24f, 0.78f, 0.12f, 2.7f, 1.9f, 0.06f};
        case DifficultyLevel::Veteran:
        default:
            return {0.06f, 1.55f, 0.26f, 0.42f, 0.20f, 0.95f, 0.14f, 3.2f, 2.2f, 0.05f};
    }
}

AIController::AIController(DifficultyLevel lvl) : Controller(), level(lvl), profile(makeProfile(lvl)) {}

AIController::AIController(float difficultyScalar) : Controller() {
    // Map scalar 0..1 to levels
    if (difficultyScalar < 0.2f) level = DifficultyLevel::VeryEasy;
    else if (difficultyScalar < 0.4f) level = DifficultyLevel::Easy;
    else if (difficultyScalar < 0.65f) level = DifficultyLevel::Medium;
    else if (difficultyScalar < 0.85f) level = DifficultyLevel::Hard;
    else level = DifficultyLevel::Veteran;
    profile = makeProfile(level);
}

Vec2 AIController::getMovementDirection(
    float dt,
    const Vec2& selfPosition,
    const Vec2& selfVelocity,
    const std::vector<std::pair<Vec2, Vec2>>& otherPlayers,
    const Vec2& arenaCenter,
    float currentArenaRadius,
    float arenaAge
) {
    (void)selfVelocity;
    (void)arenaAge;
    // Reaction-limited decisions (simulate varying skill)
    reactionAccumulator += dt;
    if (reactionAccumulator < profile.reactionLag) {
        return cachedDirection; // keep previous direction until next decision
    }
    reactionAccumulator = 0.f;

    // Timers and burst management
    decisionTimer += dt;
    if (edgeCooldown > 0.f) edgeCooldown -= dt;
    if (!burstActive) {
        // chance per second scaled by dt
        std::uniform_real_distribution<float> dist(0.f, 1.f);
        if (dist(rng) < profile.burstChance * dt) {
            burstActive = true;
            burstTimer = profile.burstDuration;
        }
    } else {
        burstTimer -= dt;
        if (burstTimer <= 0.f) burstActive = false;
    }

    // Wander noise
    wanderAngle += (profile.jitter * dt) * (rng() % 2 ? 1.f : -1.f);
    Vec2 wanderVec{std::cos(wanderAngle), std::sin(wanderAngle)};

    // Build context
    Vec2 toCenter = arenaCenter - selfPosition;
    float distFromCenter = VectorMath::magnitude(toCenter);
    float edgeDist = currentArenaRadius - distFromCenter;
    float shrinkRate = 4.0f; // matches Simulation shrinkRate
    float timeToDanger = edgeDist / std::max(0.01f, shrinkRate);

    // Target selection (nearest)
    Vec2 target{0.f, 0.f};
    Vec2 targetVel{0.f, 0.f};
    float bestDistSq = std::numeric_limits<float>::max();
    for (const auto& op : otherPlayers) {
        Vec2 d = op.first - selfPosition;
        float dsq = VectorMath::magnitudeSquared(d);
        if (dsq < bestDistSq) {
            bestDistSq = dsq;
            target = op.first;
            targetVel = op.second;
        }
    }

    // Attack vector with outward bias (push target toward boundary)
    Vec2 attack{0.f, 0.f};
    if (bestDistSq < std::numeric_limits<float>::max()) {
        Vec2 toTarget = target - selfPosition;
        Vec2 outward = target - arenaCenter; // direction to push target outwards
        Vec2 lead = targetVel * 0.25f; // simple velocity lead
        attack = VectorMath::normalize(toTarget + lead + outward * profile.attackBias);
    }

    // Edge management (only when near danger)
    Vec2 edgeVec{0.f, 0.f};
    if (edgeDist < 120.f) {
        float urgency = clampf((120.f - edgeDist) / 120.f, 0.f, 1.f);
        edgeVec = VectorMath::normalize(toCenter) * urgency;
    }

    // Shrink pre-emption
    Vec2 shrinkVec{0.f, 0.f};
    if (timeToDanger < 5.0f) {
        float u = clampf((5.0f - timeToDanger) / 5.0f, 0.f, 1.f);
        shrinkVec = VectorMath::normalize(toCenter) * u;
    }

    // Crowd handling: sidestep if many nearby
    Vec2 crowdVec{0.f, 0.f};
    int crowdCount = 0;
    Vec2 avgOffset{0.f, 0.f};
    for (const auto& op : otherPlayers) {
        Vec2 d = op.first - selfPosition;
        float dist = VectorMath::magnitude(d);
        if (dist < 140.f) {
            crowdCount++;
            avgOffset = avgOffset + d;
        }
    }
    if (crowdCount >= 3) {
        // move perpendicular to crowd vector to avoid being pinched by many
        Vec2 perp{-avgOffset.y, avgOffset.x};
        crowdVec = VectorMath::normalize(perp) * 0.25f; // keep small so collisions still happen
    }

    // Wander (small noise)
    Vec2 wander = wanderVec * profile.wanderStrength;

    // Burst scaling
    float burstScale = burstActive ? 1.3f : 1.0f;

    // Blend
    Vec2 dir = attack * (profile.aggression * burstScale)
             + edgeVec * profile.edgeFear
             + shrinkVec * profile.shrinkFear
             + crowdVec * profile.crowdAvoid
             + wander;

    // Hard survival override near boundary
    if (edgeDist < 40.f) {
        dir = VectorMath::normalize(toCenter) * 2.0f + attack * 0.5f; // prioritize staying in
    } else if (edgeDist < 70.f) {
        dir = dir + VectorMath::normalize(toCenter) * 0.8f; // bias inward while still attacking
    }

    if (VectorMath::magnitudeSquared(dir) < 0.0001f) {
        dir = VectorMath::normalize(toCenter) * 0.5f + wander;
    }

    lastDirection = VectorMath::normalize(dir);
    cachedDirection = lastDirection;
    return lastDirection;
}