#pragma once

// Global game settings
struct Settings {
    static bool leftyMode;
    
    static void toggleLeftyMode() {
        leftyMode = !leftyMode;
    }
};
