#pragma once
#include "Controller.h"

class HumanController : public Controller {
public:
    Vec2 getMovementDirection(
        float dt,
        const Vec2& selfPosition,
        const std::vector<Vec2>& otherPlayers,
        const Vec2& arenaCenter,
        float arenaRadius
    ) override;
};