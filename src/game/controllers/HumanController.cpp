#include "HumanController.h"
#include "../../core/Settings.h"
#include "../../utils/VectorMath.h"
#include <SDL2/SDL.h>
#include <cmath>
#include <vector>

Vec2 HumanController::getMovementDirection(
    float /*dt*/,
    const Vec2& /*selfPosition*/,
    const std::vector<Vec2>& /*otherPlayers*/,
    const Vec2& /*arenaCenter*/,
    float /*arenaRadius*/
) {
    Vec2 dir{0.f, 0.f};

    // Use SDL keyboard state for input
    SDL_PumpEvents();
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Check for arrow keys first (always available regardless of settings)
    bool arrowKeysUsed =
        keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_DOWN] ||
        keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_RIGHT];

    if (arrowKeysUsed) {
        // Arrow keys layout
        if (keys[SDL_SCANCODE_UP]) dir.y -= 1.f;
        if (keys[SDL_SCANCODE_DOWN]) dir.y += 1.f;
        if (keys[SDL_SCANCODE_LEFT]) dir.x -= 1.f;
        if (keys[SDL_SCANCODE_RIGHT]) dir.x += 1.f;
    } else if (Settings::leftyMode) {
        // IJKL layout for lefty mode
        if (keys[SDL_SCANCODE_I]) dir.y -= 1.f;
        if (keys[SDL_SCANCODE_K]) dir.y += 1.f;
        if (keys[SDL_SCANCODE_J]) dir.x -= 1.f;
        if (keys[SDL_SCANCODE_L]) dir.x += 1.f;
    } else {
        // WASD layout (default)
        if (keys[SDL_SCANCODE_W]) dir.y -= 1.f;
        if (keys[SDL_SCANCODE_S]) dir.y += 1.f;
        if (keys[SDL_SCANCODE_A]) dir.x -= 1.f;
        if (keys[SDL_SCANCODE_D]) dir.x += 1.f;
    }

    return VectorMath::normalize(dir);
}

