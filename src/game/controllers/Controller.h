#pragma once

#include "../../utils/VectorMath.h"
#include <vector>

class Controller {
public:
    virtual ~Controller() = default;

    virtual Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const std::vector<Vec2>& otherPlayers,
        const Vec2& arenaCenter,
        float arenaRadius
    ) = 0;
};