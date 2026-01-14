#pragma once
#include <SFML/Graphics.hpp>

class Arena {
public:
    Arena(sf::Vector2f center, float radius);

    const sf::Vector2f& getCenter() const;
    float getRadius() const;

    // Check if a position is outside the arena (more than half radius)
    bool isOutside(const sf::Vector2f& position, float playerRadius) const;

    // Render the arena to the window
    void draw(sf::RenderWindow& window) const;

private:
    sf::Vector2f center;
    float radius;
    sf::CircleShape shape;
};
