#include "KeyBindings.h"

const char* KeyBindings::getKeyName(SDL_Scancode code) {
    switch (code) {
        case SDL_SCANCODE_W: return "W";
        case SDL_SCANCODE_A: return "A";
        case SDL_SCANCODE_S: return "S";
        case SDL_SCANCODE_D: return "D";
        case SDL_SCANCODE_I: return "I";
        case SDL_SCANCODE_J: return "J";
        case SDL_SCANCODE_K: return "K";
        case SDL_SCANCODE_L: return "L";
        default: return "Unknown";
    }
}
