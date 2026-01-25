#pragma once

#include "ScreenStack.h"
#include "GraphicsContext.h"
#include "ImGuiManager.h"
#include <memory>

enum class GameState {
    MENU,
    PLAYING
};

class Game {
public:
    Game();
    void run();

private:
    std::unique_ptr<GraphicsContext> gfx;
    std::unique_ptr<ImGuiManager> imgui;
    ScreenStack screens;
    // GameState state;
};
