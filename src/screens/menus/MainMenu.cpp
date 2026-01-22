#include "MainMenu.h"
#include <cstdlib>
#include <iostream>

MainMenu::MainMenu(sf::Font& f){
    buttons.emplace_back(f, "Singleplayer", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 150.f));
    buttons.emplace_back(f, "Multiplayer", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 240.f));
    buttons.emplace_back(f, "Friends", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 330.f));
    buttons.emplace_back(f, "Lobbies", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 420.f));
    buttons.emplace_back(f, "Options", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 510.f));
    buttons.emplace_back(f, "Quit", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 600.f));
    
    // Auto-start game if running in online test mode
    if (std::getenv("SUMO_ONLINE")) {
        action = MenuAction::START_GAME;
    }
}

void MainMenu::update(sf::Time /*deltaTime*/, sf::RenderWindow& window){
    for(auto& button : buttons){
        button.update(window);
    }

    if(buttons[0].wasClicked()){
        action = MenuAction::START_SINGLEPLAYER;  // Singleplayer
        buttons[0].reset();
    }else if(buttons[1].wasClicked()){
        action = MenuAction::LOBBIES;  // Multiplayer
        buttons[1].reset();
    }else if(buttons[2].wasClicked()){
        action = MenuAction::FRIENDS;
        buttons[2].reset();
    }else if(buttons[3].wasClicked()){
        action = MenuAction::LOBBIES;
        buttons[3].reset();
    }else if(buttons[4].wasClicked()){
        action = MenuAction::OPTIONS;
        buttons[4].reset();
    }else if(buttons[5].wasClicked()){
        action = MenuAction::QUIT;
        buttons[5].reset();
    }
}

void MainMenu::render(sf::RenderWindow& window){
    for(auto& button : buttons){
        button.render(window);
    }
}
