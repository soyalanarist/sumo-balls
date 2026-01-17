#pragma once
#include "Controller.h"

class HumanController : public Controller {
public:
    sf::Vector2f getMovementDirection(
        float dt,
        const sf::Vector2f& selfPosition,
        const std::vector<sf::Vector2f>& otherPlayers,
        const sf::Vector2f& arenaCenter,
        float arenaRadius
    ) override;
};