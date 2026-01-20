#include "PlayerEntity.h"
#include <iostream>

PlayerEntity::PlayerEntity(
    sf::Vector2f startPos,
    std::unique_ptr<Controller> ctrl,
    sf::Color color
)
: player(startPos, color),
  controller(std::move(ctrl)) {}

void PlayerEntity::update(
    float dt,
    const std::vector<sf::Vector2f>& otherPlayers,
    const sf::Vector2f& arenaCenter,
    float arenaRadius,
    float speedMultiplier
) {
    sf::Vector2f dir = controller->getMovementDirection(
        dt,
        player.getPosition(),
        otherPlayers,
        arenaCenter,
        arenaRadius
    );

    player.setMovementDirection(dir);
    player.update(dt, speedMultiplier);
}

void PlayerEntity::render(sf::RenderWindow& window) {
    player.render(window);
}

sf::Vector2f PlayerEntity::getPosition() const {
    return player.getPosition();
}

float PlayerEntity::getRadius() const {
    return player.getRadius();
}

float PlayerEntity::getMass() const {
    return player.getMass();
}

sf::Vector2f PlayerEntity::getVelocity() const {
    return player.getVelocity();
}

void PlayerEntity::move(sf::Vector2f offset) {
    player.move(offset);
}

void PlayerEntity::addVelocity(sf::Vector2f impulse) {
    player.addVelocity(impulse);
}

void PlayerEntity::setMassMultiplier(float multiplier) {
    player.setMassMultiplier(multiplier);
}

bool PlayerEntity::isAlive() const {
    return player.isAlive();
}

void PlayerEntity::setAlive(bool alive) {
    player.setAlive(alive);
}
