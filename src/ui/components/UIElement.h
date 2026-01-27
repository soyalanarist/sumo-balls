#pragma once

// Legacy SFML UI - orphaned during SDL/ImGui migration
// File kept for reference but not built

class UIElement {
public:
    UIElement(sf::Vector2f position, sf::Vector2f size);
    virtual ~UIElement() = default;

    virtual void update(const sf::RenderWindow& window);
    virtual void render(sf::RenderWindow& window) = 0;

    bool wasClicked() const { return clicked; }
    bool isHovered() const { return hovered; }
    void reset() { clicked = false; }

    void setPosition(sf::Vector2f pos);
    sf::Vector2f getPosition() const { return background.getPosition(); }
    void setSize(sf::Vector2f size);
    sf::Vector2f getSize() const { return background.getSize(); }

protected:
    sf::RectangleShape background;
    bool hovered = false;
    bool clicked = false;
    bool wasMouseDownLastFrame = false;

    sf::Color normalColor = sf::Color(60, 60, 60);
    sf::Color hoverColor = sf::Color(100, 100, 100);
    sf::Color outlineColor = sf::Color::White;
};
