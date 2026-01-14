#include "Player.h"

Player::Player(sf::Vector2f startPosition, Controller* controller):
    position(startPosition),
    velocity(sf::Vector2f(0.f, 0.f)),
    movementDirection(sf::Vector2f(0.f, 0.f)),
    controller(controller),
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

void Player::resetVelocity(){
    velocity = sf::Vector2f(0.f, 0.f);
}

void Player::move(sf::Vector2f offset){
    position += offset;
    shape.setPosition(position);
}

void Player::addVelocity(sf::Vector2f impulse){
    velocity += impulse;
}

sf::Vector2f Player::getPosition() const{
    return position;
}

void Player::setPosition(sf::Vector2f newPos){
    position = newPos;
    shape.setPosition(position);
}

float Player::getRadius() const{
    return radius;
}

void Player::update(float dt)
{
    sf::Vector2f inputDir = controller->getMovementDirection(dt);
    velocity += inputDir * acceleration * dt;

    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if(speed > maxSpeed){
        velocity = (velocity / speed) * maxSpeed;
    }

    position += velocity * dt;
    shape.setPosition(position);
}

void Player::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

const sf::CircleShape& Player::getShape(){
    return shape;
}

bool Player::isAlive() const {
    return alive;
}

void Player::setAlive(bool state){
    alive = state;
}

void Player::setController(Controller* ctrl){
    controller = ctrl;
}