#include "Arena.h"
#include <cmath>

Arena::Arena(sf::Vector2f c, float r)
    : center(c), radius(r), shape(r, 100)  // 100 points for smooth circle
{
    shape.setOrigin(radius, radius); // origin at center
    shape.setPosition(center);
    shape.setFillColor(sf::Color(50, 50, 50));
    shape.setOutlineThickness(5.0f);
    shape.setOutlineColor(sf::Color::White);
}

bool Arena::contains(sf::Vector2f pos, float margin) const {
    sf::Vector2f diff = pos - center;
    float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
    return dist + margin <= radius;
}

sf::Vector2f Arena::getCenter() const {
    return center;
}

float Arena::getRadius() const {
    return radius;
}

void Arena::render(sf::RenderWindow& window) const {
    window.draw(shape);
}
