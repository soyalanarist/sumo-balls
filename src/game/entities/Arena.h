#pragma once
#include <SFML/Graphics.hpp>

class Arena {
public:
    Arena(sf::Vector2f center, float radius);

    bool contains(sf::Vector2f pos, float margin = 0.f) const;
    sf::Vector2f getCenter() const;
    float getRadius() const;
    void setRadius(float newRadius);

    void render(sf::RenderWindow& window) const;

public:
    sf::Vector2f center;
    float radius;
    sf::CircleShape shape;
};
