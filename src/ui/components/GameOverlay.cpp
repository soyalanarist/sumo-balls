#include "GameOverlay.h"
#include <cmath>
#include <string>

GameOverlay::GameOverlay(const sf::Font& f) : font(f) {
    textDisplay.setFont(font);
}

void GameOverlay::renderCountdown(sf::RenderWindow& window, float timeRemaining) {
    if (timeRemaining <= 0.f) return;
    
    int countdown = static_cast<int>(std::ceil(timeRemaining));
    std::string countdownStr = (countdown > 0) ? std::to_string(countdown) : "GO!";
    
    textDisplay.setString(countdownStr);
    textDisplay.setCharacterSize(120);
    textDisplay.setFillColor(sf::Color::Yellow);
    textDisplay.setOutlineColor(sf::Color::Black);
    textDisplay.setOutlineThickness(3.f);
    
    sf::FloatRect textBounds = textDisplay.getLocalBounds();
    textDisplay.setOrigin(
        textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f
    );
    textDisplay.setPosition(600.f, 350.f);  // Screen center
    
    window.draw(textDisplay);
}

void GameOverlay::renderGameOver(sf::RenderWindow& window, const std::string& winnerText) {
    // Dark overlay
    sf::RectangleShape overlay({1200.f, 900.f});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);
    
    // Winner text
    textDisplay.setString(winnerText);
    textDisplay.setCharacterSize(64);
    textDisplay.setFillColor(sf::Color::Yellow);
    textDisplay.setOutlineColor(sf::Color::Black);
    textDisplay.setOutlineThickness(3.f);
    
    sf::FloatRect textBounds = textDisplay.getLocalBounds();
    textDisplay.setOrigin(
        textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f
    );
    textDisplay.setPosition(600.f, 350.f);
    
    window.draw(textDisplay);
    
    // Instructions
    textDisplay.setString("Press ESC for menu");
    textDisplay.setCharacterSize(28);
    textDisplay.setFillColor(sf::Color::White);
    textDisplay.setOutlineThickness(2.f);
    
    textBounds = textDisplay.getLocalBounds();
    textDisplay.setOrigin(
        textBounds.left + textBounds.width / 2.f,
        textBounds.top + textBounds.height / 2.f
    );
    textDisplay.setPosition(600.f, 500.f);
    
    window.draw(textDisplay);
}

void GameOverlay::renderNetworkInfo(sf::RenderWindow& window, float rttMs) {
    std::string rttStr = (rttMs >= 0.f) ? std::to_string(static_cast<int>(rttMs)) + " ms" : "--";
    
    textDisplay.setString("Online  RTT: " + rttStr);
    textDisplay.setCharacterSize(18);
    textDisplay.setFillColor(sf::Color::White);
    textDisplay.setOutlineThickness(0.f);
    textDisplay.setOrigin(0.f, 0.f);
    textDisplay.setPosition(10.f, 10.f);
    
    window.draw(textDisplay);
}
