#include "UIElement.h"

UIElement::UIElement(sf::Vector2f position, sf::Vector2f size) {
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(normalColor);
    background.setOutlineThickness(2.f);
    background.setOutlineColor(outlineColor);
}

void UIElement::update(const sf::RenderWindow& window) {
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

void UIElement::setPosition(sf::Vector2f pos) {
    background.setPosition(pos);
}

void UIElement::setSize(sf::Vector2f size) {
    background.setSize(size);
}
