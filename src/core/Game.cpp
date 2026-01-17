#include "Game.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../screens/menus/MainMenu.h"
#include "../screens/menus/PauseMenu.h"
#include "../screens/menus/GameOverMenu.h"
#include "../screens/menus/MenuAction.h"

Game::Game() : screens(window, font){
    window.create({1200, 900}, "Sumo Balls");
    if(!window.isOpen()) {
        throw std::runtime_error("Failed to create game window");
    }
    
    if(!font.loadFromFile("assets/arial.ttf")) {
        throw std::runtime_error("Failed to load font: assets/arial.ttf");
    }

    screens.push(std::make_unique<MainMenu>(font));
}

void Game::run() {
    sf::Clock clock;

    while(window.isOpen()){
        sf::Time deltaTime = clock.restart();

        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed){
                window.close();
            }
        }

        try {
            screens.update(deltaTime);
        } catch(const std::exception& e) {
            window.close();
            break;
        } catch(...) {
            window.close();
            break;
        }
        
        try {
            window.clear();
            screens.render(window);
            window.display();
        } catch(const std::exception& e) {
            window.close();
            break;
        } catch(...) {
            window.close();
            break;
        }
    }
}
