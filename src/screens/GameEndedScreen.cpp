#include "GameEndedScreen.h"
#include "menus/MenuAction.h"

GameEndedScreen::GameEndedScreen(
    std::shared_ptr<GameScreen> frozenGame,
    const std::string& winner,
    const sf::Font& f
) : gameScreen(frozenGame), winnerText(winner), font(f) {}

void GameEndedScreen::update(sf::Time deltaTime, [[maybe_unused]] sf::RenderWindow& window) {
    displayTime += deltaTime.asSeconds();
    
    // After 0.7 seconds, allow returning to main menu via ESC
    if (displayTime >= 0.7f) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            returnToMenu = true;
        }
    }
}

void GameEndedScreen::render(sf::RenderWindow& window) {
    // Render frozen game state underneath
    if (gameScreen) {
        gameScreen->render(window);
    }
    
    // Draw semi-transparent overlay
    sf::RectangleShape overlay({1200.f, 900.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);
    
    // Winner text
    sf::Text winnerDisplay;
    winnerDisplay.setFont(font);
    winnerDisplay.setString(winnerText);
    winnerDisplay.setCharacterSize(64);
    winnerDisplay.setFillColor(sf::Color::Yellow);
    winnerDisplay.setOutlineColor(sf::Color::Black);
    winnerDisplay.setOutlineThickness(3.f);
    
    sf::FloatRect textBounds = winnerDisplay.getLocalBounds();
    winnerDisplay.setOrigin(
        textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f
    );
    winnerDisplay.setPosition(600.f, 350.f);
    window.draw(winnerDisplay);
    
    // Instructions (only show after brief delay)
    if (displayTime >= 0.7f) {
        sf::Text instructions;
        instructions.setFont(font);
        instructions.setString("Press ESC for Main Menu");
        instructions.setCharacterSize(28);
        instructions.setFillColor(sf::Color::White);
        instructions.setOutlineColor(sf::Color::Black);
        instructions.setOutlineThickness(2.f);
        
        textBounds = instructions.getLocalBounds();
        instructions.setOrigin(
            textBounds.left + textBounds.width / 2.f,
            textBounds.top + textBounds.height / 2.f
        );
        instructions.setPosition(600.f, 500.f);
        window.draw(instructions);
    }
}

MenuAction GameEndedScreen::getMenuAction() const {
    return returnToMenu ? MenuAction::MAIN_MENU : MenuAction::NONE;
}

void GameEndedScreen::resetMenuAction() {
    returnToMenu = false;
}
