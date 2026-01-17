#pragma once
#include <memory>
#include <vector>
#include "Screen.h"

class ScreenStack {
public:
    ScreenStack(sf::RenderWindow& window, sf::Font& font);

    void push(std::unique_ptr<Screen> screen);
    void pop();
    void clear();

    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    bool isEmpty() const { return screens.empty(); }

private:
    std::vector<std::unique_ptr<Screen>> screens;
    sf::RenderWindow& window;
    sf::Font& font;
};
