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
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> variation(-0.15f, 0.15f);
    
    // Aggressiveness: 0.3-1.0, higher difficulty = more aggressive
    aggressiveness = 0.3f + (difficulty * 0.7f) + variation(rng);
    aggressiveness = std::max(0.2f, std::min(1.0f, aggressiveness));
    
    // Caution: 0.3-1.0, higher difficulty = more cautious
    caution = 0.3f + (difficulty * 0.7f) + variation(rng);
    caution = std::max(0.2f, std::min(1.0f, caution));
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
    // Calculate distances for boundary checking
    sf::Vector2f toCenter = arenaCenter - selfPosition;
    float distToCenter = magnitude(toCenter);
    
    // CRITICAL: Boundary safety check EVERY FRAME (highest priority)
    float dangerZone = arenaRadius - (80.f + caution * 50.f);  // Cautious AIs have bigger safety margin
    
    if (distToCenter > dangerZone) {
        // Emergency return to center - override everything
        return normalize(toCenter);
    }
    
    // Update decision timer (reaction time based on difficulty)
    decisionTimer -= dt;
    float reactionTime = 0.5f - (difficulty * 0.3f);  // 0.5s (easy) to 0.2s (hard)
    
    if (decisionTimer <= 0.f) {
        decisionTimer = reactionTime;
        
        sf::Vector2f targetDirection{0.f, 0.f};
        
        if (!otherPlayers.empty()) {
            // Select best target using priority system
            int targetIdx = selectTarget(selfPosition, otherPlayers, arenaCenter);
            sf::Vector2f target = otherPlayers[targetIdx];
            sf::Vector2f toTarget = target - selfPosition;
            float distToTarget = magnitude(toTarget);
            
            // Calculate positional advantage
            sf::Vector2f targetToCenter = arenaCenter - target;
            float targetDistFromCenter = magnitude(targetToCenter);
            float advantage = calculateAdvantage(distToCenter, targetDistFromCenter);
            
            // Behavior based on advantage and personality
            if (advantage > 30.f) {
                // CHARGE MODE: We have significant positional advantage
                targetDirection = normalize(toTarget);
                
                // Add slight leading for moving targets (better at higher difficulty)
                if (difficulty > 0.5f) {
                    sf::Vector2f perpendicular(-toTarget.y, toTarget.x);
                    targetDirection = normalize(targetDirection + perpendicular * 0.1f * difficulty);
                }
                
            } else if (advantage > -30.f) {
                // PRESSURE MODE: Fairly even position
                if (distToTarget < 150.f) {
                    // Close range - be aggressive based on personality
                    if (aggressiveness > 0.6f) {
                        targetDirection = normalize(toTarget);  // Charge
                    } else {
                        // Mix approach with center positioning
                        targetDirection = normalize(toTarget * 0.7f + toCenter * 0.3f);
                    }
                } else {
                    // Medium range - close distance while maintaining center
                    targetDirection = normalize(toTarget * 0.8f + toCenter * 0.2f);
                }
                
            } else {
                // REPOSITION MODE: We're at disadvantage, get to center first
                targetDirection = normalize(toCenter);
                
                // If target is close and we're cautious, add evasive movement
                if (distToTarget < 100.f && caution > 0.6f) {
                    sf::Vector2f perpendicular(-toTarget.y, toTarget.x);
                    targetDirection = normalize(targetDirection + perpendicular * 0.3f);
                }
            }
            
        } else {
            // No opponents - move toward center and stay alert
            if (distToCenter > 50.f) {
                targetDirection = normalize(toCenter);
            } else {
                targetDirection = {0.f, 0.f};  // At center, stay still
            }
        }
        
        // Add personality-based randomness (less randomness at higher difficulty)
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        
        float jitterAmount = (1.f - difficulty) * 0.08f;
        targetDirection.x += dist(rng) * jitterAmount;
        targetDirection.y += dist(rng) * jitterAmount;
        
        cachedDirection = normalize(targetDirection);
    }
    
    return cachedDirection;
}