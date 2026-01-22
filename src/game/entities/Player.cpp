#include "Player.h"

Player::Player(sf::Vector2f startPosition, sf::Color color):
    position(startPosition),
    velocity(sf::Vector2f(0.f, 0.f)),
    movementDirection(sf::Vector2f(0.f, 0.f)),
    speed(100.f),           // Base acceleration speed
    acceleration(25.f),    // Acceleration multiplier
    friction(0.002f),        // Very low friction (~.2% decay per frame) for strong momentum
    radius(18.f)
{
    // initialize shape with 1000 points for ultra-smooth circle
    shape.setPointCount(1000);
    shape.setRadius(radius);
    shape.setOrigin(radius, radius);
    shape.setFillColor(color);
    shape.setOutlineColor(sf::Color(200, 200, 200));  // Light grey border
    shape.setOutlineThickness(2.f);
    shape.setPosition(position);
}

void Player::setMovementDirection(sf::Vector2f direction){
    movementDirection = direction;
}

void Player::update(float dt, float speedMultiplier) {
    // Clamp dt to valid range instead of skipping frame
    if(dt <= 0.f) dt = 0.016f;  // Default to ~60 FPS
    if(dt > 0.1f) dt = 0.1f;    // Cap at 100ms
    
    // Apply movement direction to velocity, scaled by speed multiplier
    velocity += movementDirection * speed * acceleration * speedMultiplier * dt;
    
    // Apply friction
    velocity *= (1.f - friction);
    
    // Cap max speed - use robust NaN/Inf detection
    float speed_magnitude = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    
    if(std::isnan(speed_magnitude) || std::isinf(speed_magnitude)) {
        velocity = sf::Vector2f(0.f, 0.f);
        return;
    }
    
    if(speed_magnitude > maxSpeed) {
        velocity = (velocity / speed_magnitude) * maxSpeed;
    }
    
    // Apply velocity to position
    position += velocity * dt;
    
    // Check for NaN in position
    if(position.x != position.x || position.y != position.y) {
        position = sf::Vector2f(400.f, 450.f);  // reset to start
    }
    
    shape.setPosition(position);
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

sf::Vector2f Player::getPosition() const {
    return position;
}

sf::Vector2f Player::getVelocity() const {
    return velocity;
}

void Player::setPosition(sf::Vector2f newPos){
    position = newPos;
    shape.setPosition(position);
}

float Player::getRadius() const {
    return radius;
}

float Player::getMass() const {
    // Mass proportional to volume (4/3 * pi * r^3)
    // Simplified: mass = k * r^3, where k is a constant
    const float DENSITY = 1.0f;  // Arbitrary unit density
    return DENSITY * radius * radius * radius * massMultiplier;
}

void Player::setMassMultiplier(float multiplier) {
    massMultiplier = multiplier;
}

void Player::render(sf::RenderWindow& window)
{
    window.draw(shape);
}

const sf::CircleShape& Player::getShape(){
    return shape;
}

sf::Color Player::getColor() const {
    return shape.getFillColor();
}

bool Player::isAlive() const {
    return alive;
}

void Player::setAlive(bool state){
    alive = state;
}
