#include "HumanController.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

static sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0.f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

sf::Vector2f HumanController::getMovementDirection(float) {
    sf::Vector2f dir{0.f, 0.f};

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;

    return normalize(dir);
}
