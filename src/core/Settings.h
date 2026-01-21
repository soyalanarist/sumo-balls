#pragma once
#include <string>
#include <SFML/Graphics.hpp>

// Global game settings
struct Settings {
    static bool leftyMode;
    static bool fullscreen;
    static int playerColorIndex;  // Index into color palette
    static bool onlineEnabled;
    static std::string onlineHost;
    static int onlinePort;
    
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
    static void setOnlineEnabled(bool enabled);
    static void setOnlineHost(const std::string& host);
    static void setOnlinePort(int port);
    
private:
    static const std::string CONFIG_FILE;
};

