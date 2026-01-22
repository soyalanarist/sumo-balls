#pragma once

/// Game phase states for proper state machine pattern
/// Replaces boolean flags with explicit state
enum class GamePhase {
    Countdown,      // Pre-game countdown (3, 2, 1, GO!)
    Playing,        // Active gameplay
    GameOver,       // Game ended, showing results
    Paused          // Game paused (for future use)
};

/// Valid state transitions for game phase
/// Prevents invalid state changes
inline bool isValidTransition(GamePhase from, GamePhase to) {
    switch (from) {
        case GamePhase::Countdown:
            return to == GamePhase::Playing || to == GamePhase::Paused;
        case GamePhase::Playing:
            return to == GamePhase::GameOver || to == GamePhase::Paused;
        case GamePhase::GameOver:
            return false; // Terminal state - must exit to menu
        case GamePhase::Paused:
            return to == GamePhase::Countdown || to == GamePhase::Playing;
    }
    return false;
}
