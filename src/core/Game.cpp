#include "Game.h"
#include <iostream>

Game::Game(){
    window.create(sf::VideoMode(1200, 900), "Sumo Balls");
    arenaCenter = sf::Vector2f(600.f, 450.f);
    arenaRadius = 300.f;
    std::cout << "Game constructed\n";
    players.emplace_back(sf::Vector2f(600.f - 100.f, 450.f));
    players.emplace_back(sf::Vector2f(600.f + 100.f, 450.f));
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

    players[0].setMovementDirection(direction);
}

void Game::update(float dt)
{
    for(auto& player : players){
        player.update(dt);

        // Distance from arena center
        sf::Vector2f diff = player.getPosition() - arenaCenter;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

        // If more than half of player radius is outside arena, trigger loss
        if (distance + player.getRadius() / 2.f > arenaRadius)
        {
            std::cout << "Player lost!\n";
            // For now, just reset player to center for testing
            player.setPosition(arenaCenter);
            player.resetVelocity();
        }
    }

    for (size_t i = 0; i < players.size(); ++i){
        for (size_t j = i + 1; j < players.size(); ++j){
            resolvePlayerCollision(players[i], players[j]);
        }
    }
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

    for(auto& player : players){
        window.draw(player.getShape());
    }
    window.display();
}

void Game::resolvePlayerCollision(Player& a, Player& b){
    sf::Vector2f diff = b.getPosition() - a.getPosition();
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    float minDist = a.getRadius() + b.getRadius();

    if (dist == 0.f || dist >= minDist)
        return;

    // Normalize collision normal
    sf::Vector2f normal = diff / dist;

    // Push players apart
    float penetration = minDist - dist;
    a.move(-normal * penetration * 0.5f);
    b.move( normal * penetration * 0.5f);

    // Knockback impulse
    float knockbackStrength = 300.f;
    a.addVelocity(-normal * knockbackStrength);
    b.addVelocity( normal * knockbackStrength);
}
