#pragma once
#include <SFML/Graphics.hpp>
#include "../game/Player.h"

class Game {
public:
    Game();
    void run();

private:
    void processInput();
    void update(float dt);
    void render();

    sf::RenderWindow window;
    Player player;
};
