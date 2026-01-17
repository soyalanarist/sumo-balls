#pragma once

#include <SFML/Graphics.hpp>

class Button {
public:
    Button(
        sf::Font& font,
        const std::string& text,
        sf::Vector2f size,
        sf::Vector2f position
    );

    void update(const sf::RenderWindow& window);
    void render(sf::RenderWindow& window);

    bool wasClicked();
    void reset();

private:
    sf::RectangleShape background;
    sf::Text label;

    bool hovered = false;
    bool clicked = false;
    bool wasMouseDownLastFrame = false;

    sf::Color normalColor = sf::Color(60, 60, 60);
    sf::Color hoverColor = sf::Color(100, 100, 100);
    sf::Color outlineColor = sf::Color::White;
};
