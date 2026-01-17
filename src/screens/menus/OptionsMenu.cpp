#include "OptionsMenu.h"
#include "../../core/Settings.h"

OptionsMenu::OptionsMenu(sf::Font& f, bool isOverlay) : overlay(isOverlay) {
    // Title
    titleText.setFont(f);
    titleText.setString("Options");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(400.f, 50.f);

    // Label for toggle switch
    labelText.setFont(f);
    labelText.setString("Lefty Mode (I/J/K/L):");
    labelText.setCharacterSize(20);
    labelText.setFillColor(sf::Color::White);
    labelText.setPosition(250.f, 280.f);

    // Status text showing current setting
    statusText.setFont(f);
    statusText.setCharacterSize(16);
    statusText.setFillColor(sf::Color::Cyan);
    statusText.setPosition(500.f, 200.f);

    // Create toggle switch
    leftyModeSwitch = std::make_unique<ToggleSwitch>(
        sf::Vector2f(550.f, 280.f),
        sf::Vector2f(100.f, 50.f),
        Settings::leftyMode
    );
    
    // Back Button
    buttons.emplace_back(f, "Back", sf::Vector2f(300.f, 70.f), 
                        sf::Vector2f(450.f, 450.f));
}

void OptionsMenu::update(sf::Time /*deltaTime*/, sf::RenderWindow& window) {
    // Update status text
    statusText.setString(Settings::leftyMode ? "Lefty Mode: ON" : "Lefty Mode: OFF");

    // Update toggle switch
    leftyModeSwitch->update(window);
    
    // Sync Settings with switch state
    if (leftyModeSwitch->isToggled() != Settings::leftyMode) {
        Settings::toggleLeftyMode();
        leftyModeSwitch->setToggled(Settings::leftyMode);
    }

    for (auto& button : buttons) {
        button.update(window);
    }

    if (buttons[0].wasClicked()) {
        // Back button behavior depends on context
        action = overlay ? MenuAction::RESUME : MenuAction::MAIN_MENU;
        buttons[0].reset();
    }
}

void OptionsMenu::render(sf::RenderWindow& window) {
    // If this is an overlay (from pause), draw blur background
    if (overlay) {
        sf::RectangleShape blurOverlay({1200.f, 900.f});
        blurOverlay.setFillColor(sf::Color(0, 0, 0, 150));  // Dark semi-transparent overlay
        window.draw(blurOverlay);
    }

    window.draw(titleText);
    window.draw(statusText);
    window.draw(labelText);
    
    leftyModeSwitch->render(window);
    
    for (auto& button : buttons) {
        button.render(window);
    }
}

