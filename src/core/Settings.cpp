#include "Settings.h"
#include "../utils/GameConstants.h"

// Static member initialization
// These mirror SettingsManager for backward compatibility
bool Settings::leftyMode = false;
bool Settings::fullscreen = true;
int Settings::playerColorIndex = 0;
bool Settings::onlineEnabled = true;  // Online is mandatory by default
std::string Settings::onlineHost = "127.0.0.1";
int Settings::onlinePort = GameConstants::DEFAULT_SERVER_PORT;
std::string Settings::authToken = "";
std::string Settings::username = "";
int Settings::userID = 0;
