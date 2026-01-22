#pragma once
#include <SFML/Graphics.hpp>
#include <string>

/// Renders game UI overlays (countdown, game over)
/// Separates UI rendering from game logic
class GameOverlay {
public:
    explicit GameOverlay(const sf::Font& font);
    ~GameOverlay() = default;
    
    /// Render countdown timer
    /// @param window: Target window
    /// @param timeRemaining: Seconds remaining in countdown
    void renderCountdown(sf::RenderWindow& window, float timeRemaining);
    
    /// Render game over screen
    /// @param window: Target window
    /// @param winnerText: Text to display (e.g., "Player 1 Wins!")
    void renderGameOver(sf::RenderWindow& window, const std::string& winnerText);
    
    /// Render online mode RTT display
    /// @param window: Target window
    /// @param rttMs: Round-trip time in milliseconds
    void renderNetworkInfo(sf::RenderWindow& window, float rttMs);

private:
    const sf::Font& font;
    sf::Text textDisplay;
};
