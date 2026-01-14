#include "Game.h"
#include <iostream>

Game::Game()
: window(sf::VideoMode(800, 600), "Sumo Balls"),
  player(sf::Vector2f(400.f, 300.f))
{
    std::cout << "Game constructed\n";
}

void Game::run()
{
    const float dt = 1.f / 60.f;

    while (window.isOpen())
    {
        processInput();
        update(dt);
        render();
    }
}

void Game::processInput()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            window.close();
    }

    sf::Vector2f direction(0.f, 0.f);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        direction.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        direction.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        direction.x += 1.f;

    player.setMovementDirection(direction);
}

void Game::update(float dt)
{
    player.update(dt);
}

void Game::render()
{
    window.clear(sf::Color::Blue);
    player.draw(window);
    window.display();
}
