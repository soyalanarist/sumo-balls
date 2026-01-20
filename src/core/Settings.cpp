#include "Settings.h"
#include <fstream>
#include <sstream>
#include <filesystem>

bool Settings::leftyMode = false;
bool Settings::fullscreen = true;
int Settings::playerColorIndex = 0;  // Default to white
const std::string Settings::CONFIG_FILE = "config.json";

sf::Color Settings::getPlayerColor() {
    static const sf::Color colors[] = {
        sf::Color::White,
        sf::Color::Red,
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color::Yellow,
        sf::Color::Magenta,
        sf::Color::Cyan,
        sf::Color(255, 128, 0),  // Orange
        sf::Color(128, 0, 128),  // Purple
        sf::Color(255, 192, 203) // Pink
    };
    
    int index = playerColorIndex;
    if (index < 0 || index >= 10) index = 0;
    return colors[index];
}

const char* Settings::getColorName(int index) {
    static const char* names[] = {
        "White", "Red", "Green", "Blue", "Yellow",
        "Magenta", "Cyan", "Orange", "Purple", "Pink"
    };
    if (index < 0 || index >= 10) return "White";
    return names[index];
}

int Settings::getColorCount() {
    return 10;
}

void Settings::loadSettings() {
    std::ifstream file(CONFIG_FILE);
    if (!file.is_open()) {
        // File doesn't exist, use defaults and save them
        saveSettings();
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    std::string content = buffer.str();
    
    // Simple JSON parsing
    // Look for "leftyMode": true/false
    size_t pos = content.find("\"leftyMode\"");
    if (pos != std::string::npos) {
        size_t colonPos = content.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t truePos = content.find("true", colonPos);
            size_t falsePos = content.find("false", colonPos);
            
            if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
                leftyMode = true;
            } else {
                leftyMode = false;
            }
        }
    }
    
    // Look for "fullscreen": true/false
    pos = content.find("\"fullscreen\"");
    if (pos != std::string::npos) {
        size_t colonPos = content.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t truePos = content.find("true", colonPos);
            size_t falsePos = content.find("false", colonPos);
            
            if (truePos != std::string::npos && (falsePos == std::string::npos || truePos < falsePos)) {
                fullscreen = true;
            } else {
                fullscreen = false;
            }
        }
    }
    
    // Look for "playerColorIndex": number
    pos = content.find("\"playerColorIndex\"");
    if (pos != std::string::npos) {
        size_t colonPos = content.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t numStart = colonPos + 1;
            while (numStart < content.size() && (content[numStart] == ' ' || content[numStart] == '\t' || content[numStart] == '\n')) {
                numStart++;
            }
            if (numStart < content.size() && std::isdigit(content[numStart])) {
                playerColorIndex = std::stoi(content.substr(numStart));
            }
        }
    }
}

void Settings::saveSettings() {
    std::ofstream file(CONFIG_FILE);
    if (!file.is_open()) {
        return;  // Failed to open file
    }
    
    file << "{\n";
    file << "  \"leftyMode\": " << (leftyMode ? "true" : "false") << ",\n";
    file << "  \"fullscreen\": " << (fullscreen ? "true" : "false") << ",\n";
    file << "  \"playerColorIndex\": " << playerColorIndex << "\n";
    file << "}\n";
    
    file.close();
}

