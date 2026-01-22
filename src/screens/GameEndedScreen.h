#pragma once
#include "../core/Screen.h"
#include "GameScreen.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

/// Overlay screen for game over state
/// Wraps the frozen game view and displays winner information
/// Separates game-over UI from active gameplay logic
class GameEndedScreen : public Screen {
public:
    /// Constructor
    /// @param frozenGame: The game screen to display underneath (frozen)
    /// @param winnerText: Text to display (e.g., "Player 1 Wins!")
    /// @param font: Font for rendering text
    GameEndedScreen(
        std::shared_ptr<GameScreen> frozenGame,
        const std::string& winnerText,
        const sf::Font& font
    );
    
    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return true; }
    
    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

private:
    std::shared_ptr<GameScreen> gameScreen;  // Frozen game state
    std::string winnerText;
    const sf::Font& font;
    float displayTime = 0.f;
    bool returnToMenu = false;
};
