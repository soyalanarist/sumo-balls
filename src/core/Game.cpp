#include "Game.h"
#include "Settings.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../screens/AuthScreen.h"
#include "../screens/menus/MainMenu.h"
#include "../screens/menus/PauseMenu.h"
#include "../screens/menus/GameOverMenu.h"
#include "../screens/menus/MenuAction.h"
#include <iostream>

Game::Game() : screens(window, font){
    // Load settings from config file
    Settings::loadSettings();
    
    // Enable anti-aliasing for smooth circles (request higher level)
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16;  // Request 16x; driver may negotiate down
    
    // Create window based on fullscreen setting
    if (Settings::fullscreen) {
        window.create(sf::VideoMode::getDesktopMode(), "Sumo Balls", sf::Style::Fullscreen, settings);
    } else {
        window.create(sf::VideoMode(1200, 900), "Sumo Balls", sf::Style::Default, settings);
        // Position window on primary monitor (top-left of primary display)
        window.setPosition(sf::Vector2i(0, 0));
    }
    
    if(!window.isOpen()) {
        throw std::runtime_error("Failed to create game window");
    }
    
    if(!font.loadFromFile("assets/arial.ttf")) {
        throw std::runtime_error("Failed to load font: assets/arial.ttf");
    }

    // Set frame rate limit to 60 FPS for smooth gameplay
    window.setFramerateLimit(60);

    // If not authenticated, show auth screen first
    if(Settings::authToken.empty()) {
        screens.push(std::make_unique<AuthScreen>(font));
    } else {
        // Already authenticated, go to main menu
        screens.push(std::make_unique<MainMenu>(font));
    }
}

void Game::run() {
    sf::Clock clock;
    std::cout << "[Game] Starting main loop" << std::endl;

    int frameCount = 0;
    while(window.isOpen()){
        frameCount++;
        sf::Time deltaTime = clock.restart();

        try {
            if(frameCount == 1) std::cout << "[Game] Frame 1: calling screens.update()" << std::endl;
            screens.update(deltaTime);
            if(frameCount == 1) std::cout << "[Game] Frame 1: screens.update() completed" << std::endl;
        } catch(const std::exception& e) {
            std::cerr << "Game update error: " << e.what() << std::endl;
            window.close();
            break;
        } catch(...) {
            std::cerr << "Unknown game update error occurred" << std::endl;
            window.close();
            break;
        }
        
        try {
            window.clear(sf::Color::Black);
            screens.render(window);
            window.display();
        } catch(const std::exception& e) {
            std::cerr << "Game render error: " << e.what() << std::endl;
            window.close();
            break;
        } catch(...) {
            std::cerr << "Unknown game render error occurred" << std::endl;
            window.close();
            break;
        }
    }
}
