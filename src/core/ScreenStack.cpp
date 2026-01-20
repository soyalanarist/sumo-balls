#include "ScreenStack.h"
#include "Settings.h"
#include "../screens/menus/MainMenu.h"
#include "../screens/menus/OptionsMenu.h"
#include "../screens/menus/PauseMenu.h"
#include "../screens/menus/MenuAction.h"
#include "../screens/GameScreen.h"
#include <SFML/Graphics.hpp>
#include <iostream>

ScreenStack::ScreenStack(sf::RenderWindow& w, sf::Font& f)
    : window(w), font(f){}

void ScreenStack::push(std::unique_ptr<Screen> screen) {
    screens.push_back(std::move(screen));
}

void ScreenStack::pop(){
    if(!screens.empty()){
        screens.pop_back();
    }
}

void ScreenStack::clear(){
    while(!screens.empty()){
        screens.pop_back();
    }
}

void ScreenStack::update(sf::Time deltaTime) {
    if (screens.empty()) {
        return;
    }

    // Update from top until blocked
    for(int i = static_cast<int>(screens.size()) - 1; i >= 0; i--){
        screens[i]->update(deltaTime, window);
        if(!screens[i]->isOverlay()){
            break;
        }
    }

    // Handle menu actions from the top screen using virtual interface
    MenuAction action = screens.back()->getMenuAction();
    
    if(action != MenuAction::NONE) {
        switch(action){
            case MenuAction::START_GAME:
                pop();
                push(std::make_unique<GameScreen>());
                break;
            case MenuAction::OPTIONS: {
                // Check if we're in a game (overlay) or in menu
                bool isGameOverlay = screens.back()->isOverlay();
                push(std::make_unique<OptionsMenu>(font, isGameOverlay));
                break;
            }
            case MenuAction::QUIT:
                window.close();
                break;
            case MenuAction::MAIN_MENU:
                while(screens.size() > 1) pop();
                pop();
                push(std::make_unique<MainMenu>(font));
                break;
            case MenuAction::RESUME:
                pop();  // Remove pause menu
                break;
            case MenuAction::PAUSE:
                push(std::make_unique<PauseMenu>(font));
                break;
            case MenuAction::SET_WINDOWED:
                window.create(sf::VideoMode(1200, 900), "Sumo Balls");
                window.setPosition(sf::Vector2i(0, 0));  // Position on primary monitor
                Settings::setFullscreen(false);
                break;
            case MenuAction::SET_FULLSCREEN:
                window.create(sf::VideoMode::getDesktopMode(), "Sumo Balls", sf::Style::Fullscreen);
                Settings::setFullscreen(true);
                break;
            case MenuAction::TOGGLE_FULLSCREEN:
            case MenuAction::NONE:
            default:
                break;
        }
        
        screens.back()->resetMenuAction();
    }
}

void ScreenStack::render(sf::RenderWindow& window) {
    // Render from bottom up, but skip screens that are underneath non-overlay screens
    for(int i = 0; i < static_cast<int>(screens.size()); i++){
        // Check if any screen above this one is non-overlay
        bool hasNonOverlayAbove = false;
        for(int j = i + 1; j < static_cast<int>(screens.size()); j++){
            if(!screens[j]->isOverlay()){
                hasNonOverlayAbove = true;
                break;
            }
        }
        
        // Only skip this screen if there's a non-overlay screen above it
        if(!hasNonOverlayAbove){
            screens[i]->render(window);
        }
    }
}
