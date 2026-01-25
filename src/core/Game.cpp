#include "Game.h"
#include "Settings.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../screens/WelcomeScreen.h"
#include "../screens/menus/MainMenu.h"
#include "../screens/menus/PauseMenu.h"
#include "../screens/menus/GameOverMenu.h"
#include "../screens/menus/MenuAction.h"
#include <iostream>

Game::Game() : screens(nullptr, nullptr) {
    // Load settings from config file
    Settings::loadSettings();
    
    // Initialize graphics context
    gfx = std::make_unique<GraphicsContext>();
    if (!gfx->initialize("Sumo Balls", 1200, 900, Settings::fullscreen)) {
        throw std::runtime_error("Failed to initialize graphics context");
    }
    
    // Initialize ImGui
    imgui = std::make_unique<ImGuiManager>();
    if (!imgui->initialize(gfx.get())) {
        throw std::runtime_error("Failed to initialize ImGui");
    }
    
    std::cout << "[Game] Graphics and UI initialized" << std::endl;

    // If not authenticated, show auth screen first
    if(Settings::authToken.empty()) {
        screens.push(std::make_unique<WelcomeScreen>());
    } else {
        // Already authenticated, go to main menu
        screens.push(std::make_unique<MainMenu>());
    }
}

void Game::run() {
    std::cout << "[Game] Starting main loop" << std::endl;

    int frameCount = 0;
    while(gfx->isRunning()) {
        frameCount++;
        
        // Handle window events and pass to ImGui
        if (gfx->processEvents(imgui.get())) {
            break;
        }

        try {
            // Update screens (no longer takes deltaTime or window)
            screens.update();
            
        } catch(const std::exception& e) {
            std::cerr << "Game update error: " << e.what() << std::endl;
            break;
        } catch(...) {
            std::cerr << "Unknown game update error occurred" << std::endl;
            break;
        }
        
        try {
            // Render frame
            gfx->beginFrame();
            
            imgui->beginFrame();
            screens.render();  // No longer takes window parameter
            imgui->endFrame(gfx.get());
            
            gfx->endFrame();
            
        } catch(const std::exception& e) {
            std::cerr << "Game render error: " << e.what() << std::endl;
            break;
        } catch(...) {
            std::cerr << "Unknown game render error occurred" << std::endl;
            break;
        }
    }
    
    std::cout << "[Game] Main loop ended" << std::endl;
}
