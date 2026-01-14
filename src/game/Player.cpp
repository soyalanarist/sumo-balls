#include "Player.h"

Player::Player(sf::Vector2f startPosition):
    position(startPosition),
    velocity(sf::Vector2f(0.f, 0.f)),
    movementDirection(sf::Vector2f(0.f, 0.f)),
    speed(30.f),
    acceleration(5.f),
    friction(1.f / 10.f),
    radius(15.f)
{
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(sf::Color::White);
    shape.setPosition(position);
}

void Player::setMovementDirection(sf::Vector2f direction){
    movementDirection = direction;
}

sf::Vector2f Player::getPosition() const{
    return position;
}

float Player::getRadius() const{
    return radius;
}

void Player::setPosition(sf::Vector2f newPos){
    position = newPos;
    shape.setPosition(position);
}

void Player::update(float dt)
{
    // Desired velocity
    sf::Vector2f desiredVelocity = movementDirection * speed;

    // Compute difference
    sf::Vector2f diff = desiredVelocity - velocity;

    // Compute change limited by acceleration
    if (diff.x != 0.f)
    {
        float accelX = std::min(std::abs(diff.x), acceleration * dt);
        velocity.x += (diff.x > 0 ? accelX : -accelX);
    }
    if (diff.y != 0.f)
    {
        float accelY = std::min(std::abs(diff.y), acceleration * dt);
        velocity.y += (diff.y > 0 ? accelY : -accelY);
    }

    // Apply friction when no input
    if (movementDirection.x == 0.f)
    {
        float frictionX = std::min(std::abs(velocity.x), friction * dt);
        velocity.x += (velocity.x > 0 ? -frictionX : frictionX);
    }
    if (movementDirection.y == 0.f)
    {
        float frictionY = std::min(std::abs(velocity.y), friction * dt);
        velocity.y += (velocity.y > 0 ? -frictionY : frictionY);
    }

    // Update position
    position += velocity * dt;
    shape.setPosition(position);
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}
