#include "ToggleSwitch.h"
#include <cmath>

ToggleSwitch::ToggleSwitch(
    sf::Vector2f position,
    sf::Vector2f size,
    bool initialState
) : UIElement(position, size), toggled(initialState), animationProgress(initialState ? 1.f : 0.f) {
    normalColor = sf::Color(80, 80, 80);
    hoverColor = sf::Color(120, 120, 120);
}

void ToggleSwitch::update(const sf::RenderWindow& window) {
    // Call parent update for click/hover detection
    UIElement::update(window);

    // Handle toggle on click
    if (clicked) {
        toggled = !toggled;
    }

    // Animate towards target state
    float targetProgress = toggled ? 1.f : 0.f;
    if (std::abs(animationProgress - targetProgress) > 0.01f) {
        animationProgress += (targetProgress - animationProgress) * ANIMATION_SPEED;
    } else {
        animationProgress = targetProgress;
    }
}

void ToggleSwitch::render(sf::RenderWindow& window) {
    // Draw background track
    window.draw(background);

    // Draw toggle circle that animates
    float switchWidth = background.getSize().x;
    float switchHeight = background.getSize().y;
    float circleRadius = switchHeight * 0.35f;
    
    sf::Vector2f bgPos = background.getPosition();
    
    // Position of the circle: slides from left to right
    float circleX = bgPos.x + circleRadius + (switchWidth - circleRadius * 2.f) * animationProgress;
    float circleY = bgPos.y + switchHeight / 2.f;
    
    // Draw animated circle
    sf::CircleShape toggleCircle(circleRadius);
    toggleCircle.setFillColor(toggled ? sf::Color::Green : sf::Color(180, 180, 180));
    toggleCircle.setPosition(circleX - circleRadius, circleY - circleRadius);
    
    window.draw(toggleCircle);

    // Draw ON/OFF labels
    // This is simple text rendering (you could add font parameter later for more customization)
}
