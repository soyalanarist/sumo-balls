#include "MainMenu.h"

MainMenu::MainMenu(sf::Font& f){
    buttons.emplace_back(f, "Start Game", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 300.f));
    buttons.emplace_back(f, "Options", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 400.f));
    buttons.emplace_back(f, "Quit", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 500.f));
}

void MainMenu::update(sf::Time /*deltaTime*/, sf::RenderWindow& window){
    for(auto& button : buttons){
        button.update(window);
    }

    if(buttons[0].wasClicked()){
        action = MenuAction::START_GAME;
        buttons[0].reset();
    }else if(buttons[1].wasClicked()){
        action = MenuAction::OPTIONS;
        buttons[1].reset();
    }else if(buttons[2].wasClicked()){
        action = MenuAction::QUIT;
        buttons[2].reset();
    }
}

void MainMenu::render(sf::RenderWindow& window){
    for(auto& button : buttons){
        button.render(window);
    }
}
