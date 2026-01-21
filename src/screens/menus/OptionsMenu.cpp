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
    labelText.setPosition(250.f, 200.f);

    // Status text showing current setting
    statusText.setFont(f);
    statusText.setCharacterSize(16);
    statusText.setFillColor(sf::Color::Cyan);
    statusText.setPosition(250.f, 150.f);

    // Create toggle switch
    leftyModeSwitch = std::make_unique<ToggleSwitch>(
        sf::Vector2f(550.f, 200.f),
        sf::Vector2f(100.f, 50.f),
        Settings::leftyMode
    );
    
    // Display Mode Label
    displayModeLabel.setFont(f);
    displayModeLabel.setString("Display Mode:");
    displayModeLabel.setCharacterSize(20);
    displayModeLabel.setFillColor(sf::Color::White);
    displayModeLabel.setPosition(250.f, 270.f);
    
    // Windowed Button
    buttons.emplace_back(f, "Windowed", sf::Vector2f(150.f, 50.f), 
                        sf::Vector2f(300.f, 300.f));
    
    // Fullscreen Button
    buttons.emplace_back(f, "Fullscreen", sf::Vector2f(150.f, 50.f), 
                        sf::Vector2f(550.f, 300.f));
    
    // Color Label
    colorLabel.setFont(f);
    colorLabel.setString("Player Color:");
    colorLabel.setCharacterSize(20);
    colorLabel.setFillColor(sf::Color::White);
    colorLabel.setPosition(250.f, 370.f);
    
    // Color preview text
    colorPreview.setFont(f);
    colorPreview.setCharacterSize(18);
    colorPreview.setPosition(250.f, 400.f);

    // Online label/status
    onlineLabel.setFont(f);
    onlineLabel.setString("Online Multiplayer:");
    onlineLabel.setCharacterSize(20);
    onlineLabel.setFillColor(sf::Color::White);
    onlineLabel.setPosition(250.f, 470.f);

    onlineStatus.setFont(f);
    onlineStatus.setCharacterSize(16);
    onlineStatus.setFillColor(sf::Color::Cyan);
    onlineStatus.setPosition(250.f, 440.f);

    onlineSwitch = std::make_unique<ToggleSwitch>(
        sf::Vector2f(550.f, 470.f),
        sf::Vector2f(100.f, 50.f),
        Settings::onlineEnabled
    );
    
    // Color selection buttons (Previous/Next)
    colorButtonsStartIndex = static_cast<int>(buttons.size());
    buttons.emplace_back(f, "< Previous", sf::Vector2f(140.f, 50.f), 
                        sf::Vector2f(400.f, 400.f));
    
    buttons.emplace_back(f, "Next >", sf::Vector2f(140.f, 50.f), 
                        sf::Vector2f(650.f, 400.f));
    
    // Back Button
    buttons.emplace_back(f, "Back", sf::Vector2f(300.f, 70.f), 
                        sf::Vector2f(450.f, 700.f));
}

void OptionsMenu::update(sf::Time /*deltaTime*/, sf::RenderWindow& window) {
    // Update status text
    statusText.setString(Settings::leftyMode ? "Lefty Mode: ON" : "Lefty Mode: OFF");
    onlineStatus.setString(Settings::onlineEnabled ? "Online: ON" : "Online: OFF");
    
    // Update color preview
    colorPreview.setString(Settings::getColorName(Settings::playerColorIndex));
    colorPreview.setFillColor(Settings::getPlayerColor());

    // Update toggle switch
    leftyModeSwitch->update(window);
    onlineSwitch->update(window);
    
    // Sync Settings with switch state
    if (leftyModeSwitch->isToggled() != Settings::leftyMode) {
        Settings::toggleLeftyMode();
        leftyModeSwitch->setToggled(Settings::leftyMode);
    }

    if (onlineSwitch->isToggled() != Settings::onlineEnabled) {
        Settings::setOnlineEnabled(onlineSwitch->isToggled());
    }

    for (auto& button : buttons) {
        button.update(window);
    }

    // Button 0: Windowed
    if (buttons[0].wasClicked()) {
        action = MenuAction::SET_WINDOWED;
        buttons[0].reset();
    }
    
    // Button 1: Fullscreen
    if (buttons[1].wasClicked()) {
        action = MenuAction::SET_FULLSCREEN;
        buttons[1].reset();
    }
    
    // Button 2 (colorButtonsStartIndex): Previous Color
    if (buttons[colorButtonsStartIndex].wasClicked()) {
        int newIndex = Settings::playerColorIndex - 1;
        if (newIndex < 0) newIndex = Settings::getColorCount() - 1;
        Settings::setPlayerColor(newIndex);
        buttons[colorButtonsStartIndex].reset();
    }
    
    // Button 3 (colorButtonsStartIndex+1): Next Color
    if (buttons[colorButtonsStartIndex + 1].wasClicked()) {
        int newIndex = (Settings::playerColorIndex + 1) % Settings::getColorCount();
        Settings::setPlayerColor(newIndex);
        buttons[colorButtonsStartIndex + 1].reset();
    }
    
    // Last Button: Back
    if (buttons.back().wasClicked()) {
        // Back button behavior depends on context
        action = overlay ? MenuAction::RESUME : MenuAction::MAIN_MENU;
        buttons.back().reset();
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
    window.draw(displayModeLabel);
    window.draw(colorLabel);
    window.draw(colorPreview);
    window.draw(onlineLabel);
    window.draw(onlineStatus);
    
    leftyModeSwitch->render(window);
    onlineSwitch->render(window);
    
    for (auto& button : buttons) {
        button.render(window);
    }
}

