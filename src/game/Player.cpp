#include "Player.h"

Player::Player(sf::Vector2f startPosition)
: position(startPosition),
  velocity(0.f, 0.f),
  movementDirection(0.f, 0.f),
  speed(90.f),
  radius(12.f)
{
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(sf::Color::White);
    shape.setPosition(position);
}

void Player::setMovementDirection(sf::Vector2f direction)
{
    movementDirection = direction;
}

void Player::update(float dt)
{
    velocity = movementDirection * speed;
    position += velocity * dt;
    shape.setPosition(position);
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}
