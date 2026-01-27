#include "../TestFramework.h"
#include "../../src/core/Settings.h"

TEST(Settings, StaticMembers) {
    // Test that Settings has static members properly defined
    TEST_TRUE(Settings::playerColorIndex >= 0);
    TEST_ASSERT(Settings::onlinePort > 0, "online port should be set");
    return true;
}

TEST(Settings, ColorCount) {
    // Settings should have defined colors
    int colorCount = Settings::getColorCount();
    TEST_ASSERT(colorCount > 0, "Should have at least one color");
    return true;
}

TEST(Settings, ColorIndexRange) {
    // Valid color index should get a valid name
    for (int i = 0; i < Settings::getColorCount(); i++) {
        const char* name = Settings::getColorName(i);
        TEST_ASSERT(name != nullptr, "Color name should not be null");
    }
    return true;
}

TEST(Settings, GetPlayerColor) {
    // Should be able to get player color without crash
    sf::Color color = Settings::getPlayerColor();
    // Just verify we can call it
    return true;
}

TEST(Settings, SetPlayerColor) {
    // Set a valid color index
    int originalColor = Settings::playerColorIndex;
    Settings::setPlayerColor(0);
    TEST_EQUAL(0, Settings::playerColorIndex, "Color should be set to 0");
    
    // Restore
    Settings::setPlayerColor(originalColor);
    return true;
}

TEST(Settings, ToggleLeftyMode) {
    bool originalLefty = Settings::leftyMode;
    Settings::toggleLeftyMode();
    TEST_EQUAL(!originalLefty, Settings::leftyMode, "Lefty mode should toggle");
    
    // Restore
    Settings::toggleLeftyMode();
    TEST_EQUAL(originalLefty, Settings::leftyMode, "Lefty mode should be restored");
    return true;
}

TEST(Settings, SetFullscreen) {
    bool originalFullscreen = Settings::fullscreen;
    Settings::setFullscreen(true);
    TEST_EQUAL(true, Settings::fullscreen, "Fullscreen should be true");
    
    Settings::setFullscreen(false);
    TEST_EQUAL(false, Settings::fullscreen, "Fullscreen should be false");
    
    // Restore
    Settings::setFullscreen(originalFullscreen);
    return true;
}
