#pragma once
#include <memory>
#include <vector>
#include "Screen.h"

class ScreenStack {
public:
    ScreenStack(void* window, void* font);  // Keep for compatibility, unused now

    void push(std::unique_ptr<Screen> screen);
    void pop();
    void clear();

    void update();  // No longer takes deltaTime or window
    void render();  // No longer takes window

    bool isEmpty() const { return screens.empty(); }

private:
    std::vector<std::unique_ptr<Screen>> screens;
};
