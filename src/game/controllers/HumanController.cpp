#include "HumanController.h"
#include "../../core/Settings.h"
#include "../../core/KeyBindings.h"
#include "../../utils/VectorMath.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

Vec2 HumanController::getMovementDirection(
    float /*dt*/,
    const Vec2& /*selfPosition*/,
    const Vec2& /*selfVelocity*/,
    const std::vector<std::pair<Vec2, Vec2>>& /*otherPlayers*/,
    const Vec2& /*arenaCenter*/,
    float /*currentArenaRadius*/,
    float /*arenaAge*/
) {
    Vec2 dir{0.f, 0.f};

    // Use SDL keyboard state for input
    SDL_PumpEvents();
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Get keybindings based on handedness
    SDL_Scancode upKey = KeyBindings::getMoveUpKey(Settings::leftyMode);
    SDL_Scancode downKey = KeyBindings::getMoveDownKey(Settings::leftyMode);
    SDL_Scancode leftKey = KeyBindings::getMoveLeftKey(Settings::leftyMode);
    SDL_Scancode rightKey = KeyBindings::getMoveRightKey(Settings::leftyMode);
    
    // Check movement keys
    if (keys[upKey]) dir.y -= 1.f;
    if (keys[downKey]) dir.y += 1.f;
    if (keys[leftKey]) dir.x -= 1.f;
    if (keys[rightKey]) dir.x += 1.f;

    return VectorMath::normalize(dir);
}

