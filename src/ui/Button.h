#pragma once

#include "UIElement.h"
#include <string>

class Button : public UIElement {
public:
    Button(
        sf::Font& font,
        const std::string& text,
        sf::Vector2f size,
        sf::Vector2f position,
        const std::string& tooltip = ""
    );

    void render(sf::RenderWindow& window) override;

private:
    sf::Text label;
    sf::Text tooltipText;
    std::string tooltipContent;
};

