#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

class Player {
public:
    Player(sf::Vector2f startPosition);

    void update(float dt);
    void draw(sf::RenderWindow& window);
    const sf::CircleShape& getShape();

    void setMovementDirection(sf::Vector2f direction);
    void resetVelocity();
    void move(sf::Vector2f offset);
    void addVelocity(sf::Vector2f impulse);

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f newPos);
    float getRadius() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f movementDirection;

    sf::CircleShape shape;

    float speed;
    float acceleration;
    float friction;
    float radius;

    bool alive = true;
};
