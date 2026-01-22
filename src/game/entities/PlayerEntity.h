#pragma once
#include "Player.h"
#include "../controllers/Controller.h"
#include <memory>
#include <vector>

class PlayerEntity {
public:
    PlayerEntity(
        sf::Vector2f startPos,
        std::unique_ptr<Controller> controller,
        sf::Color color = sf::Color::White
    );
    
    // Move semantics
    PlayerEntity(PlayerEntity&& other) noexcept = default;
    PlayerEntity& operator=(PlayerEntity&& other) noexcept = default;
    
    // Delete copy semantics since we have unique_ptr
    PlayerEntity(const PlayerEntity&) = delete;
    PlayerEntity& operator=(const PlayerEntity&) = delete;

    void update(
        float dt,
        const std::vector<sf::Vector2f>& otherPlayers,
        const sf::Vector2f& arenaCenter,
        float arenaRadius,
        float speedMultiplier = 1.0f
    );

    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    float getRadius() const;
    float getMass() const;
    sf::Color getColor() const { return player.getColor(); }
    sf::Vector2f getVelocity() const;
    void move(sf::Vector2f offset);
    void addVelocity(sf::Vector2f impulse);
    void setMassMultiplier(float multiplier);  // For gameplay balance

    bool isAlive() const;
    void setAlive(bool alive);

private:
    Player player;
    std::unique_ptr<Controller> controller;
};
