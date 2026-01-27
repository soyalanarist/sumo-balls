#include "Button.h"

Button::Button(
    sf::Font& font,
    const std::string& text,
    sf::Vector2f size,
    sf::Vector2f position,
    const std::string& tooltip
) : UIElement(position, size), tooltipContent(tooltip) {
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

    // Setup tooltip text
    tooltipText.setFont(font);
    tooltipText.setString(tooltip);
    tooltipText.setCharacterSize(14);
    tooltipText.setFillColor(sf::Color::Yellow);
}

void Button::render(sf::RenderWindow& window) {
    window.draw(background);
    window.draw(label);

    // Draw tooltip if hovered and tooltip exists
    if (hovered && !tooltipContent.empty()) {
        sf::Vector2f buttonPos = background.getPosition();
        float tooltipX = buttonPos.x + background.getSize().x / 2.f - 100.f;
        float tooltipY = buttonPos.y - 60.f;

        tooltipText.setPosition(tooltipX, tooltipY);
        
        // Draw background for tooltip
        sf::RectangleShape tooltipBg({200.f, 50.f});
        tooltipBg.setPosition(tooltipX - 5.f, tooltipY - 5.f);
        tooltipBg.setFillColor(sf::Color(30, 30, 30, 200));
        tooltipBg.setOutlineThickness(1.f);
        tooltipBg.setOutlineColor(sf::Color::Yellow);
        
        window.draw(tooltipBg);
        window.draw(tooltipText);
    }
}
