#include "AIController.h"

AIController::AIController(float difficultyLevel) : Controller(), difficulty(difficultyLevel) {}

Vec2 AIController::getMovementDirection(
    float /*dt*/, const Vec2& /*selfPosition*/,
    const std::vector<Vec2>& /*otherPlayers*/, const Vec2& /*arenaCenter*/, float /*arenaRadius*/) {
    return {0.f, 0.f};
}