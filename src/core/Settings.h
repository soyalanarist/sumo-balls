#pragma once
#include <string>
#include <SFML/Graphics.hpp>

// Global game settings
struct Settings {
    static bool leftyMode;
    static bool fullscreen;
    static int playerColorIndex;  // Index into color palette
    
    static void toggleLeftyMode() {
        leftyMode = !leftyMode;
        saveSettings();
    }
    
    static void setFullscreen(bool fs) {
        fullscreen = fs;
        saveSettings();
    }
    
    static void setPlayerColor(int colorIndex) {
        playerColorIndex = colorIndex;
        saveSettings();
    }
    
    static sf::Color getPlayerColor();
    static const char* getColorName(int index);
    static int getColorCount();
    
    static void loadSettings();
    static void saveSettings();
    
private:
    static const std::string CONFIG_FILE;
};

