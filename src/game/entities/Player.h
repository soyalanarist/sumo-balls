#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

class Player {
public:
    Player(sf::Vector2f startPosition, sf::Color color = sf::Color::White);

    void update(float dt);
    void render(sf::RenderWindow& window);
    const sf::CircleShape& getShape();

    void setMovementDirection(sf::Vector2f direction);
    void resetVelocity();
    void move(sf::Vector2f offset);
    void addVelocity(sf::Vector2f impulse);

    sf::Vector2f getPosition() const;
    sf::Vector2f getVelocity() const;
    void setPosition(sf::Vector2f newPos);
    float getRadius() const;
    float getMass() const;  // Mass based on radius (volume)
    
    bool isAlive() const;
    void setAlive(bool state);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f movementDirection;

    float speed;
    float maxSpeed = 500.f;   // Max velocity reachable through acceleration
    float acceleration;
    float friction;
    float radius;

    bool alive = true;

    sf::CircleShape shape;
};
