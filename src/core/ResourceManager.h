#pragma once

#include <memory>
#include <stdexcept>
#include <string>

// Legacy SFML resource manager - orphaned during SDL/ImGui migration
// File kept for reference but not built

/// Enterprise-grade resource manager with shared ownership semantics
/// Manages lifetime of window, fonts, and other shared game resources
/// Thread-safe through shared_ptr reference counting
class ResourceManager {
public:
    /// Shared pointer to the main render window
    std::shared_ptr<sf::RenderWindow> window;
    
    /// Shared pointer to the default font
    std::shared_ptr<sf::Font> defaultFont;
    
    /// Factory method to create a fully initialized ResourceManager
    /// @param windowTitle Title for the game window
    /// @param windowWidth Width of the window in pixels
    /// @param windowHeight Height of the window in pixels
    /// @param fullscreen Whether to create fullscreen window
    /// @param fontPath Path to the default font file
    /// @return Shared pointer to initialized ResourceManager
    /// @throws std::runtime_error if window or font creation fails
    static std::shared_ptr<ResourceManager> create(
        const std::string& windowTitle = "Sumo Balls",
        unsigned int windowWidth = 1200,
        unsigned int windowHeight = 900,
        bool fullscreen = false,
        const std::string& fontPath = "assets/arial.ttf"
    );
    
    /// Check if the window is still open and valid
    bool isWindowOpen() const {
        return window && window->isOpen();
    }
    
    /// Get the window size
    sf::Vector2u getWindowSize() const {
        if (!window) {
            throw std::runtime_error("Window not initialized");
        }
        return window->getSize();
    }
    
    /// Get reference to window (throws if not initialized)
    sf::RenderWindow& getWindow() {
        if (!window) {
            throw std::runtime_error("Window not initialized");
        }
        return *window;
    }
    
    /// Get reference to font (throws if not initialized)
    sf::Font& getFont() {
        if (!defaultFont) {
            throw std::runtime_error("Font not initialized");
        }
        return *defaultFont;
    }

private:
    /// Private constructor - use factory method instead
    ResourceManager() = default;
};
