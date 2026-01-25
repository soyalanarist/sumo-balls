#pragma once
#include <string>
#include <memory>
#include "../utils/GameConstants.h"

/// Manages game settings with validation and persistence
/// Singleton pattern for easier migration from static Settings class
/// TODO: Convert to dependency injection when refactoring architecture
class SettingsManager {
public:
    /// Get singleton instance
    static SettingsManager& instance();
    
    // Delete copy/move constructors (singleton)
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    SettingsManager(SettingsManager&&) = delete;
    SettingsManager& operator=(SettingsManager&&) = delete;
    
    // Accessors with validation
    bool isLeftyMode() const { return leftyMode; }
    bool isFullscreen() const { return fullscreen; }
    int getPlayerColorIndex() const { return playerColorIndex; }
    bool isOnlineEnabled() const { return onlineEnabled; }
    std::string getOnlineHost() const { return onlineHost; }
    int getOnlinePort() const { return onlinePort; }
    
    // Setters with validation
    void setLeftyMode(bool enabled);
    void toggleLeftyMode();
    void setFullscreen(bool fs);
    void setPlayerColorIndex(int index);
    void setOnlineEnabled(bool enabled);
    void setOnlineHost(const std::string& host);
    void setOnlinePort(int port);
    
    // Persistence
    void load();
    void save();
    
    // Reset to defaults
    void resetToDefaults();
    
private:
    SettingsManager();
    ~SettingsManager() = default;
    
    /// Validate and clamp color index to valid range
    int validateColorIndex(int index) const;
    
    /// Validate port number
    int validatePort(int port) const;
    
    // Settings data
    bool leftyMode;
    bool fullscreen;
    int playerColorIndex;
    bool onlineEnabled;
    std::string onlineHost;
    int onlinePort;
    
    static const std::string CONFIG_FILE;
    
    // Validation constants
    static constexpr int MIN_PORT = 1024;
    static constexpr int MAX_PORT = 65535;
};
