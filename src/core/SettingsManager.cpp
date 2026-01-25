#include "SettingsManager.h"
#include "../utils/SimpleJson.h"
#include "../utils/GameConstants.h"
#include <fstream>
#include <iostream>
#include <algorithm>

const std::string SettingsManager::CONFIG_FILE = "config.json";

SettingsManager& SettingsManager::instance() {
    static SettingsManager manager;
    return manager;
}

SettingsManager::SettingsManager()
    : leftyMode(false)
    , fullscreen(true)
    , playerColorIndex(0)
    , onlineEnabled(true)  // Force online as the default mode
    , onlineHost("127.0.0.1")
    , onlinePort(GameConstants::DEFAULT_SERVER_PORT)
{
}

void SettingsManager::setLeftyMode(bool enabled) {
    leftyMode = enabled;
    save();
}

void SettingsManager::toggleLeftyMode() {
    leftyMode = !leftyMode;
    save();
}

void SettingsManager::setFullscreen(bool fs) {
    fullscreen = fs;
    save();
}

void SettingsManager::setPlayerColorIndex(int index) {
    playerColorIndex = validateColorIndex(index);
    save();
}

void SettingsManager::setOnlineEnabled(bool enabled) {
    onlineEnabled = enabled;
    save();
}

void SettingsManager::setOnlineHost(const std::string& host) {
    if (host.empty()) {
        std::cerr << "[Settings Warning] Empty host provided, using localhost" << std::endl;
        onlineHost = "127.0.0.1";
    } else {
        onlineHost = host;
    }
    save();
}

void SettingsManager::setOnlinePort(int port) {
    onlinePort = validatePort(port);
    save();
}

int SettingsManager::validateColorIndex(int index) const {
    if (index != 0) {
        std::cerr << "[Settings Warning] Invalid color index " << index 
                  << ", clamping to 0" << std::endl;
        return 0;
    }
    return index;
}

int SettingsManager::validatePort(int port) const {
    if (port < MIN_PORT || port > MAX_PORT) {
        std::cerr << "[Settings Warning] Invalid port " << port 
                  << ", clamping to [" << MIN_PORT << "," << MAX_PORT << "]" << std::endl;
        return std::clamp(port, MIN_PORT, MAX_PORT);
    }
    return port;
}

void SettingsManager::load() {
    std::ifstream file(CONFIG_FILE);
    if (!file) {
        std::cout << "[Settings] Config file not found, using defaults" << std::endl;
        return;
    }
    
    std::string jsonContent((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    file.close();
    
    SimpleJson json = SimpleJson::parse(jsonContent);
    
    // Load with validation
    if (json.has("leftyMode")) {
        leftyMode = json.getBool("leftyMode");
    }
    
    if (json.has("fullscreen")) {
        fullscreen = json.getBool("fullscreen");
    }
    
    if (json.has("playerColor")) {
        int loadedIndex = json.getInt("playerColor");
        playerColorIndex = validateColorIndex(loadedIndex);
        if (loadedIndex != playerColorIndex) {
            std::cerr << "[Settings] Corrected invalid playerColor " << loadedIndex 
                      << " -> " << playerColorIndex << std::endl;
        }
    }
    
    if (json.has("onlineEnabled")) {
        onlineEnabled = json.getBool("onlineEnabled");
    }
    
    if (json.has("onlineHost")) {
        std::string host = json.getString("onlineHost");
        if (!host.empty()) {
            onlineHost = host;
        }
    }
    
    if (json.has("onlinePort")) {
        int loadedPort = json.getInt("onlinePort");
        onlinePort = validatePort(loadedPort);
        if (loadedPort != onlinePort) {
            std::cerr << "[Settings] Corrected invalid onlinePort " << loadedPort 
                      << " -> " << onlinePort << std::endl;
        }
    }
    
    std::cout << "[Settings] Loaded from " << CONFIG_FILE << std::endl;
}

void SettingsManager::save() {
    SimpleJson json;
    json.set("leftyMode", leftyMode);
    json.set("fullscreen", fullscreen);
    json.set("playerColor", playerColorIndex);
    json.set("onlineEnabled", onlineEnabled);
    json.set("onlineHost", onlineHost);
    json.set("onlinePort", onlinePort);
    
    std::ofstream file(CONFIG_FILE);
    if (!file) {
        std::cerr << "[Settings Error] Failed to write config file" << std::endl;
        return;
    }
    
    file << json.toString();
    file.close();
}

void SettingsManager::resetToDefaults() {
    leftyMode = false;
    fullscreen = true;
    playerColorIndex = 0;
    onlineEnabled = false;
    onlineHost = "127.0.0.1";
    onlinePort = GameConstants::DEFAULT_SERVER_PORT;
    save();
    std::cout << "[Settings] Reset to defaults" << std::endl;
}
