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

AIController::AIController(float diff) : difficulty(diff) {
    // Generate personality traits based on difficulty with some randomization
    std::uniform_real_distribution<float> variation(-0.15f, 0.15f);
    
    // Aggressiveness: 0.3-1.0, higher difficulty = more aggressive
    aggressiveness = 0.3f + (difficulty * 0.7f) + variation(rng);
    aggressiveness = std::max(0.2f, std::min(1.0f, aggressiveness));
    
    // Caution: 0.3-1.0, higher difficulty = more cautious
    caution = 0.3f + (difficulty * 0.7f) + variation(rng);
    caution = std::max(0.2f, std::min(1.0f, caution));

    // Pick a persistent tangential patrol direction to avoid flip-flopping
    std::uniform_int_distribution<int> signDist(0, 1);
    patrolSign = signDist(rng) ? 1 : -1;
    lastDirection = {0.f, 0.f};
    // Initialize wander angle
    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    wanderAngle = angleDist(rng);
    // Initialize burst schedule with a random initial delay
    std::uniform_real_distribution<float> burstDelayDist(6.0f, 14.0f);
    nextBurstDelay = burstDelayDist(rng);
}

float AIController::calculateAdvantage(float selfDist, float opponentDist) const {
    // Positive = we have advantage (more centered)
    // Negative = opponent has advantage
    return opponentDist - selfDist;
}

int AIController::selectTarget(const sf::Vector2f& selfPosition, 
                                const std::vector<sf::Vector2f>& opponents,
                                const sf::Vector2f& arenaCenter) const {
    if (opponents.empty()) return -1;
    if (opponents.size() == 1) return 0;
    
    int bestTarget = 0;
    float bestScore = -999999.f;
    
    for (size_t i = 0; i < opponents.size(); i++) {
        float score = 0.f;
        sf::Vector2f toOpponent = opponents[i] - selfPosition;
        float distToOpponent = magnitude(toOpponent);
        
        sf::Vector2f opponentToCenter = arenaCenter - opponents[i];
        float opponentDistFromCenter = magnitude(opponentToCenter);
        
        // Priority 1: CRITICAL - Anyone very close (immediate threat or opportunity)
        if (distToOpponent < 80.f) {
            score += 200.f;
        }
        
        // Priority 2: OPPORTUNISTIC - Target vulnerable players near edge
        if (opponentDistFromCenter > 180.f) {
            score += 150.f * (opponentDistFromCenter / 300.f);
        }
        
        // Priority 3: PROXIMITY - Closer targets are easier to engage
        score += (300.f - distToOpponent) * 0.3f;
        
        // Aggressive personalities prefer closest target
        score += (1.0f - aggressiveness) * (300.f - distToOpponent) * 0.2f;
        
        if (score > bestScore) {
            bestScore = score;
            bestTarget = static_cast<int>(i);
        }
    }
    
    return bestTarget;
}

sf::Vector2f AIController::getMovementDirection(
    float dt,
    const sf::Vector2f& selfPosition,
    const std::vector<sf::Vector2f>& otherPlayers,
    const sf::Vector2f& arenaCenter,
    float arenaRadius
) {
    sf::Vector2f toCenter = arenaCenter - selfPosition;
    float distToCenter = magnitude(toCenter);

    float dangerZone = arenaRadius - (30.f + caution * 20.f);
    edgeCooldown = std::max(0.f, edgeCooldown - dt);
    if (distToCenter > dangerZone) {
        std::uniform_real_distribution<float> panicError(-0.08f, 0.08f);
        sf::Vector2f panicDirection = normalize(toCenter);
        panicDirection.x += panicError(rng);
        panicDirection.y += panicError(rng);
        lastDirection = normalize(panicDirection);
        cachedDirection = lastDirection;
        return cachedDirection;
    }

    // Update burst scheduling
    if (burstActive) {
        burstTimer -= dt;
        if (burstTimer <= 0.f) {
            burstActive = false;
        }
    } else {
        nextBurstDelay -= dt;
        if (nextBurstDelay <= 0.f) {
            burstActive = true;
            burstTimer = 5.0f;
            std::uniform_real_distribution<float> burstDelayDist(6.0f, 14.0f);
            nextBurstDelay = burstDelayDist(rng);
        }
    }

    decisionTimer -= dt;
    float reactionTimeBase = 1.0f - (difficulty * 0.3f);
    float reactionTime = std::max(0.5f, reactionTimeBase - (burstActive ? 0.2f : 0.0f));
    if (decisionTimer <= 0.f) {
        decisionTimer = reactionTime;
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        std::uniform_real_distribution<float> mistakeChance(0.0f, 1.0f);
        sf::Vector2f targetDirection{0.f, 0.f};
        float errorRate = (1.f - difficulty) * 0.6f;
        bool makeMistake = mistakeChance(rng) < errorRate;

        if (!otherPlayers.empty()) {
            int targetIdx = selectTarget(selfPosition, otherPlayers, arenaCenter);
            if (makeMistake && otherPlayers.size() > 1) {
                std::uniform_int_distribution<int> wrongTarget(0, static_cast<int>(otherPlayers.size()) - 1);
                targetIdx = wrongTarget(rng);
            }

            sf::Vector2f target = otherPlayers[targetIdx];
            sf::Vector2f toTarget = target - selfPosition;
            float distToTarget = magnitude(toTarget);
            sf::Vector2f targetToCenter = arenaCenter - target;
            float targetDistFromCenter = magnitude(targetToCenter);
            float advantage = calculateAdvantage(distToCenter, targetDistFromCenter);

            float effAgg = std::min(1.0f, aggressiveness + (burstActive ? 0.25f : 0.0f));
            float effCaution = std::max(0.1f, caution - (burstActive ? 0.15f : 0.0f));
            if (advantage > 30.f) {
                targetDirection = normalize(toTarget);
                if (difficulty > 0.5f && !makeMistake) {
                    sf::Vector2f perpendicular(-toTarget.y, toTarget.x);
                    targetDirection = normalize(targetDirection + perpendicular * 0.1f * difficulty);
                }
            } else if (advantage > -30.f) {
                if (distToTarget < 170.f || burstActive) {
                    if (effAgg > 0.5f) {
                        targetDirection = normalize(toTarget);
                    } else {
                        targetDirection = normalize(toTarget * 0.88f + toCenter * 0.12f);
                    }
                } else {
                    targetDirection = normalize(toTarget * 0.95f + toCenter * 0.05f);
                }
            } else {
                targetDirection = normalize(toCenter * 0.25f + toTarget * 0.75f);
                if (distToTarget < 100.f && effCaution > 0.6f) {
                    sf::Vector2f perpendicular(-toTarget.y, toTarget.x);
                    targetDirection = normalize(targetDirection + perpendicular * 0.25f);
                }
            }

            sf::Vector2f outward = normalize(selfPosition - arenaCenter);
            float preferredRing = arenaRadius * 0.82f;
            float ringMargin = 40.f + (1.f - difficulty) * 50.f;
            if (distToCenter < preferredRing - ringMargin) {
                float outwardBias = (burstActive ? 0.12f : 0.2f) + (1.f - difficulty) * 0.08f;
                targetDirection = normalize(targetDirection + outward * outwardBias);
            } else if (distToCenter > preferredRing + ringMargin) {
                float inwardBias = (burstActive ? 0.02f : 0.05f);
                targetDirection = normalize(targetDirection + normalize(toCenter) * inwardBias);
            }
            if (mistakeChance(rng) < 0.005f) patrolSign *= -1;
        } else {
            sf::Vector2f outward = normalize(selfPosition - arenaCenter);
            float preferredRing = arenaRadius * 0.82f;
            float ringMargin = 40.f + (1.f - difficulty) * 50.f;
            if (distToCenter < preferredRing - ringMargin) {
                targetDirection = normalize(outward * (burstActive ? 0.6f : 0.7f) + toCenter * (burstActive ? 0.4f : 0.3f));
            } else if (distToCenter > preferredRing + ringMargin) {
                targetDirection = normalize(toCenter);
            } else {
                sf::Vector2f tangent(-outward.y, outward.x);
                float tangentGain = (0.55f + (1.f - difficulty) * 0.15f) + (burstActive ? 0.1f : 0.0f);
                targetDirection = normalize(tangent * tangentGain * static_cast<float>(patrolSign));
            }
        }

        // Add curved wander to avoid straight-line traversals
        {
            // Update wander angle with low-frequency noise
            float angleDelta = dist(rng) * wanderJitter * dt;
            wanderAngle += angleDelta;
            sf::Vector2f wanderVec(std::cos(wanderAngle), std::sin(wanderAngle));
            // Perpendicular component to desired direction for gentle arcs
            sf::Vector2f perp(-targetDirection.y, targetDirection.x);
            float edgeFactorW = std::min(1.f, std::max(0.f, (arenaRadius - distToCenter) / (arenaRadius * 0.25f)));
            float wanderScale = wanderStrength * (burstActive ? 0.5f : 1.0f) * (1.f - 0.7f * edgeFactorW);
            float curvaGain = (burstActive ? 0.08f : 0.12f) * (1.f - 0.6f * edgeFactorW);
            targetDirection = normalize(targetDirection + perp * curvaGain + wanderVec * wanderScale);
        }

        float edgeFactor = std::min(1.f, std::max(0.f, (arenaRadius - distToCenter) / (arenaRadius * 0.25f)));
        float jitterBase = (1.f - difficulty) * (burstActive ? 0.6f : 0.7f);
        float jitterAmount = jitterBase * (1.f - edgeFactor);
        targetDirection.x += dist(rng) * jitterAmount;
        targetDirection.y += dist(rng) * jitterAmount;

        if (edgeCooldown <= 0.f && mistakeChance(rng) < (0.35f * (1.f - difficulty)) * (1.f - edgeFactor)) {
            float overCorrect = 1.6f + dist(rng) * 1.0f;
            targetDirection.x *= overCorrect;
            targetDirection.y *= overCorrect;
        }
        if (edgeCooldown <= 0.f && mistakeChance(rng) < (0.3f * (1.f - difficulty)) * (1.f - edgeFactor)) {
            float underCorrect = 0.3f + dist(rng) * 0.3f;
            targetDirection.x *= underCorrect;
            targetDirection.y *= underCorrect;
        }
        if (edgeCooldown <= 0.f && mistakeChance(rng) < (0.2f * (1.f - difficulty)) * (1.f - edgeFactor)) {
            targetDirection = {0.f, 0.f};
        }

        float smoothFactor = (edgeCooldown > 0.f) ? 0.97f : (burstActive ? 0.85f : 0.9f);
        sf::Vector2f blended = cachedDirection * smoothFactor + targetDirection * (1.f - smoothFactor);
        if (blended.x == 0.f && blended.y == 0.f) blended = cachedDirection;
        cachedDirection = normalize(blended);
        lastDirection = cachedDirection;
    }

    return cachedDirection;
}