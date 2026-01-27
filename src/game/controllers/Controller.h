#pragma once

#include "../../utils/VectorMath.h"
#include <vector>
#include <utility>

class Controller {
public:
    virtual ~Controller() = default;

    virtual Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const Vec2& selfVelocity,
        const std::vector<std::pair<Vec2, Vec2>>& otherPlayers, // pos, vel
        const Vec2& arenaCenter,
        float currentArenaRadius,
        float arenaAge
    ) = 0;
};