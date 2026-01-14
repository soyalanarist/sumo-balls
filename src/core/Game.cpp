#include "Game.h"
#include <iostream>
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"

Game::Game(){
    window.create(sf::VideoMode(1200, 900), "Sumo Balls");
    arenaCenter = sf::Vector2f(600.f, 450.f);
    arenaRadius = 300.f;
    std::cout << "Game constructed\n";

    players.emplace_back(sf::Vector2f(arenaCenter.x - 2 * 15, arenaCenter.y - 2 * 15), nullptr); // Human player
    players.emplace_back(sf::Vector2f(arenaCenter.x + 2 * 15, arenaCenter.y + 2 * 15), nullptr); // AI player 1

    controllers.push_back(std::make_unique<HumanController>());
    players[0].setController(controllers[0].get());

    controllers.push_back(std::make_unique<AIController>(&players[1], &players, arenaCenter, arenaRadius));
    players[1].setController(controllers[1].get());
}

void Game::run(){
    const float dt = 1.f / 60.f;

    while (window.isOpen())
    {
        processInput();
        update(dt);
        render();
    }
}

void Game::processInput(){
    sf::Event event;
    while (window.pollEvent(event)){
        if(event.type == sf::Event::Closed){
            window.close();
        }
    }
}

void Game::update(float dt){
    for(auto& player : players){
        if(!player.isAlive()){continue;}

        player.update(dt);

        // Distance from arena center
        sf::Vector2f toCenter = player.getPosition() - arenaCenter;
        float distanceToCenter = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);

        // if more than half of player radius is outside arena, trigger loss
        if(distanceToCenter + player.getRadius() > arenaRadius){
            std::cout << "Player lost!\n";
            player.setAlive(false);
            player.resetVelocity();
        }
    }

    for(size_t i = 0; i < players.size(); ++i){
        for(size_t j = i + 1; j < players.size(); ++j){
            resolvePlayerCollision(players[i], players[j]);
        }
    }
}

void Game::render(){
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
    float knockbackStrength = 100.f;
    a.addVelocity(-normal * knockbackStrength);
    b.addVelocity( normal * knockbackStrength);
}
