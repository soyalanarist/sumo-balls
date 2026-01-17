#pragma once

#include <SFML/System/Vector2.hpp>
#include <vector>

class Controller {
public:
    virtual ~Controller() = default;

    virtual sf::Vector2f getMovementDirection(
        float dt,
        const sf::Vector2f& selfPosition,
        const std::vector<sf::Vector2f>& otherPlayers,
        const sf::Vector2f& arenaCenter,
        float arenaRadius
    ) = 0;
};