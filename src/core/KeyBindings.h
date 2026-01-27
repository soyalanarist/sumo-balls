#pragma once

#include <SDL2/SDL.h>

// Keybindings configuration
class KeyBindings {
public:
    // Movement keys - Right-handed (WASD)
    static constexpr SDL_Scancode MOVE_UP_RIGHTY = SDL_SCANCODE_W;
    static constexpr SDL_Scancode MOVE_DOWN_RIGHTY = SDL_SCANCODE_S;
    static constexpr SDL_Scancode MOVE_LEFT_RIGHTY = SDL_SCANCODE_A;
    static constexpr SDL_Scancode MOVE_RIGHT_RIGHTY = SDL_SCANCODE_D;
    
    // Movement keys - Left-handed (IJKL)
    static constexpr SDL_Scancode MOVE_UP_LEFTY = SDL_SCANCODE_I;
    static constexpr SDL_Scancode MOVE_DOWN_LEFTY = SDL_SCANCODE_K;
    static constexpr SDL_Scancode MOVE_LEFT_LEFTY = SDL_SCANCODE_J;
    static constexpr SDL_Scancode MOVE_RIGHT_LEFTY = SDL_SCANCODE_L;
    
    // Helper functions
    static SDL_Scancode getMoveUpKey(bool leftHanded) {
        return leftHanded ? MOVE_UP_LEFTY : MOVE_UP_RIGHTY;
    }
    
    static SDL_Scancode getMoveDownKey(bool leftHanded) {
        return leftHanded ? MOVE_DOWN_LEFTY : MOVE_DOWN_RIGHTY;
    }
    
    static SDL_Scancode getMoveLeftKey(bool leftHanded) {
        return leftHanded ? MOVE_LEFT_LEFTY : MOVE_LEFT_RIGHTY;
    }
    
    static SDL_Scancode getMoveRightKey(bool leftHanded) {
        return leftHanded ? MOVE_RIGHT_LEFTY : MOVE_RIGHT_RIGHTY;
    }
    
    // Get key name as string for UI display
    static const char* getKeyName(SDL_Scancode code);
};
