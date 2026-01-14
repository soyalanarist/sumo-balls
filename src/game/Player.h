#pragma once
#include <SFML/Graphics.hpp>

class Player {
public:
    Player(sf::Vector2f startPosition);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    void setMovementDirection(sf::Vector2f direction);

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f movementDirection;

    float speed;
    float radius;

    sf::CircleShape shape;
};
