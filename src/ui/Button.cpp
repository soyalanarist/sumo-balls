#include "Button.h"

Button::Button(
    sf::Font& font,
    const std::string& text,
    sf::Vector2f size,
    sf::Vector2f position
) {
    background.setSize(size);
    background.setPosition(position);
    background.setFillColor(normalColor);
    background.setOutlineThickness(2.f);
    background.setOutlineColor(outlineColor);

    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(24);
    label.setFillColor(sf::Color::White);

    // center text inside button
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setOrigin(
        textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f
    );
    label.setPosition(
        position.x + size.x / 2.f,
        position.y + size.y / 2.f
    );
}

void Button::update(const sf::RenderWindow& window) {
    hovered = false;
    clicked = false;

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld(
        static_cast<float>(mousePos.x),
        static_cast<float>(mousePos.y)
    );

    bool isMouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    if (background.getGlobalBounds().contains(mouseWorld)) {
        hovered = true;

        // Click happens when mouse button transitions from down to up while hovering
        if (wasMouseDownLastFrame && !isMouseDown) {
            clicked = true;
        }
    }

    wasMouseDownLastFrame = isMouseDown;
    background.setFillColor(hovered ? hoverColor : normalColor);
}

void Button::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(label);
}

bool Button::wasClicked() {
    return clicked;
}

void Button::reset() {
    clicked = false;
}
