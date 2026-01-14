#pragma once

#include "Controller.h"

class HumanController : public Controller {
public:
    sf::Vector2f getMovementDirection(float dt) override;
};
