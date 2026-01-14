#pragma once

#include <SFML/System/Vector2.hpp>

class Controller {
public:
    virtual ~Controller() = default;

    // Returns a normalized movement direction (or {0,0})
    virtual sf::Vector2f getMovementDirection(float dt) = 0;
};