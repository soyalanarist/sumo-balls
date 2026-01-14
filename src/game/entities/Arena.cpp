#include "Arena.h"
#include <cmath>

Arena::Arena(sf::Vector2f center, float radius)
    : center(center), radius(radius), shape(radius)
{
    shape.setOrigin(radius, radius);
    shape.setPosition(center);
    shape.setFillColor(sf::Color(50, 50, 50));
    shape.setOutlineThickness(5.f);
    shape.setOutlineColor(sf::Color::White);
}

const sf::Vector2f& Arena::getCenter() const {
    return center;
}

float Arena::getRadius() const {
    return radius;
}

bool Arena::isOutside(const sf::Vector2f& position, float playerRadius) const {
    sf::Vector2f diff = position - center;
    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    return (distance + playerRadius > radius);
}

void Arena::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}
