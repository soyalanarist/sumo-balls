#include "Game.h"
#include <iostream>
#include <cmath>

Game::Game()
: window(sf::VideoMode(1200, 900), "Sumo Balls"),
  player(sf::Vector2f(400.f, 300.f))
{
    arenaCenter = sf::Vector2f(600.f, 450.f);
    arenaRadius = 300.f;
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

    // normalize vector if not zero - prevent faster diagonal movement
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length != 0.f)
        direction /= length;

    player.setMovementDirection(direction);
}

void Game::update(float dt)
{
    player.update(dt);
}

void Game::render()
{
    window.clear(sf::Color::Black);

    sf::CircleShape arenaShape(arenaRadius);
    arenaShape.setOrigin(arenaRadius, arenaRadius);
    arenaShape.setPosition(arenaCenter);
    arenaShape.setFillColor(sf::Color(50, 50, 50));
    arenaShape.setOutlineThickness(5.f);
    arenaShape.setOutlineColor(sf::Color::White);
    window.draw(arenaShape);

    player.draw(window);
    window.display();
}
