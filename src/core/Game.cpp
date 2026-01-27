#include "Game.h"
#include "Settings.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "ui/views/LoginView.h"
#include "ui/views/MainMenuView.h"
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

    // Wire up graphics context to screen stack for fullscreen toggle
    screens.setGraphicsContext(gfx.get());

    // If not authenticated, show auth screen first
    if(Settings::authToken.empty()) {
        screens.push(std::make_unique<LoginView>());
    } else {
        // Already authenticated, go to main menu
        screens.push(std::make_unique<MainMenuView>());
    }
}

void Game::run() {
    std::cout << "[Game] Starting main loop" << std::endl;

    int frameCount = 0;
    while(gfx->isRunning()) {
        frameCount++;
        
        // Handle window events and pass to ImGui and screens
        SDL_Event event;
        bool shouldQuit = false;
        while (SDL_PollEvent(&event)) {
            // Let ImGui handle first
            imgui->processEvent(&event);
            
            // Let screens handle input
            bool handled = screens.handleInput(event);
            
            // Global handlers for unhandled events
            if (!handled) {
                if (event.type == SDL_QUIT) {
                    shouldQuit = true;
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                    // ESC quits if no screen consumed it (e.g., at main menu)
                    shouldQuit = true;
                } else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    shouldQuit = true;
                }
            }
        }
        
        if (shouldQuit) break;

        try {
            // Update screens (no longer takes deltaTime or window)
            screens.update();
            if (screens.isEmpty()) break;
            
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
