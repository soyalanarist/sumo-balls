// AIController.h
#pragma once
#include "Controller.h"

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
};