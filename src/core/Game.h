#pragma once

#include <SFML/Graphics.hpp>
#include "../game/Player.h"
#include <vector>
#include <memory>

class Game {
public:
    Game();
    void run();

private:
    void processInput();
    void update(float dt);
    void render();

    void resolvePlayerCollision(Player& a, Player& b);

    std::vector<Player> players;
    std::vector<std::unique_ptr<Controller>> controllers;

    sf::RenderWindow window;
    sf::Vector2f arenaCenter;
    float arenaRadius;
};
