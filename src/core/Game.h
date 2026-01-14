#pragma once

#include <SFML/Graphics.hpp>
#include "../game/entities/Player.h"
#include "../game/entities/Arena.h"
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

    sf::RenderWindow window;

    Arena arena;

    std::vector<Player> players;
    std::vector<std::unique_ptr<Controller>> controllers;
};
