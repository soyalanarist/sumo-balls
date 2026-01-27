#pragma once
#include <memory>
#include <vector>
#include "Screen.h"

class GraphicsContext;  // Forward declaration
union SDL_Event;  // Forward declaration

class ScreenStack {
public:
    ScreenStack(void* window, void* font);  // Keep for compatibility, unused now

    void push(std::unique_ptr<Screen> screen);
    void pop();
    void clear();

    void update();  // No longer takes deltaTime or window
    void render();  // No longer takes window
    bool handleInput(const SDL_Event& event);  // Process input through screens

    bool isEmpty() const { return screens.empty(); }
    
    void setGraphicsContext(GraphicsContext* gfx) { graphics = gfx; }

private:
    std::vector<std::unique_ptr<Screen>> screens;
    GraphicsContext* graphics = nullptr;
};
