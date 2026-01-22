#pragma once
#include "SettingsManager.h"

// Legacy static interface - delegates to SettingsManager
// TODO: Migrate all code to use SettingsManager directly, then remove this
struct Settings {
    static bool leftyMode;
    static bool fullscreen;
    static int playerColorIndex;
    static bool onlineEnabled;
    static std::string onlineHost;
    static int onlinePort;
    static std::string authToken;      // Auth token from coordinator
    static std::string username;       // Current logged-in username
    static int userID;                 // Current logged-in user ID
    
    static void toggleLeftyMode() {
        SettingsManager::instance().toggleLeftyMode();
        syncFromManager();
    }
    
    static void setFullscreen(bool fs) {
        SettingsManager::instance().setFullscreen(fs);
        syncFromManager();
    }
    
    static void setPlayerColor(int colorIndex) {
        SettingsManager::instance().setPlayerColorIndex(colorIndex);
        syncFromManager();
    }
    
    static sf::Color getPlayerColor() {
        return SettingsManager::instance().getPlayerColor();
    }
    
    static const char* getColorName(int index) {
        return SettingsManager::instance().getColorName(index);
    }
    
    static int getColorCount() {
        return SettingsManager::instance().getColorCount();
    }
    
    static void loadSettings() {
        SettingsManager::instance().load();
        syncFromManager();
    }
    
    static void saveSettings() {
        // Already saved by SettingsManager
    }
    
    static void setOnlineEnabled(bool enabled) {
        SettingsManager::instance().setOnlineEnabled(enabled);
        syncFromManager();
    }
    
    static void setOnlineHost(const std::string& host) {
        SettingsManager::instance().setOnlineHost(host);
        syncFromManager();
    }
    
    static void setOnlinePort(int port) {
        SettingsManager::instance().setOnlinePort(port);
        syncFromManager();
    }
    
private:
    // Sync static vars from manager for backward compat
    static void syncFromManager() {
        auto& mgr = SettingsManager::instance();
        leftyMode = mgr.isLeftyMode();
        fullscreen = mgr.isFullscreen();
        playerColorIndex = mgr.getPlayerColorIndex();
        onlineEnabled = mgr.isOnlineEnabled();
        onlineHost = mgr.getOnlineHost();
        onlinePort = mgr.getOnlinePort();
    }
};


