#pragma once

#include "UIElement.h"

class ToggleSwitch : public UIElement {
public:
    ToggleSwitch(
        sf::Vector2f position,
        sf::Vector2f size,
        bool initialState = false
    );

    void update(const sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

    bool isToggled() const { return toggled; }
    void setToggled(bool state) { toggled = state; }

private:
    bool toggled;
    float animationProgress = 0.f;  // 0.0 = off, 1.0 = on
    const float ANIMATION_SPEED = 0.1f;  // How fast the switch animates
};
