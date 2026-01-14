#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>
#include "../controllers/Controller.h"

class Player {
public:
    Player(sf::Vector2f startPosition, Controller* controller);

    void update(float dt);
    void draw(sf::RenderWindow& window);
    const sf::CircleShape& getShape();

    void setController(Controller* ctrl);

    void setMovementDirection(sf::Vector2f direction);
    void resetVelocity();
    void move(sf::Vector2f offset);
    void addVelocity(sf::Vector2f impulse);

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f newPos);
    float getRadius() const;

    bool isAlive() const;
    void setAlive(bool state);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f movementDirection;

    Controller* controller;

    float speed;
    float maxSpeed = 30.f;
    float acceleration;
    float friction;
    float radius;

    bool alive = true;
    sf::CircleShape shape;
};
