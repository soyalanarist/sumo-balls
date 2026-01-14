#include "Game.h"
#include <iostream>

Game::Game()
: window(sf::VideoMode(800, 600), "Sumo Balls")
{
    std::cout << "Game constructed\n";
}

void Game::run()
{
    std::cout << "Entering game loop\n";

    const float dt = 1.f / 60.f;

    while (window.isOpen())
    {
        processInput();
        update(dt);
        render();
    }

    std::cout << "Exiting game loop\n";
}

void Game::processInput()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
    }
}

void Game::update(float)
{
    // No gameplay yet
}

void Game::render()
{
    window.clear(sf::Color::Blue);
    window.display();
}
