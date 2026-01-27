#pragma once
#include "Controller.h"

class HumanController : public Controller {
public:
    Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const Vec2& selfVelocity,
        const std::vector<std::pair<Vec2, Vec2>>& otherPlayers,
        const Vec2& arenaCenter,
        float currentArenaRadius,
        float arenaAge
    ) override;
};