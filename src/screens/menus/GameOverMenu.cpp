#include "GameOverMenu.h"

GameOverMenu::GameOverMenu(sf::Font& f){
    buttons.emplace_back(f, "Restart", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 300.f));
    buttons.emplace_back(f, "Main Menu", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 400.f));
    buttons.emplace_back(f, "Quit", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 500.f));

    mask.setSize(sf::Vector2f(1200.f, 900.f));
    mask.setFillColor(sf::Color(0, 0, 0, 150));
}

void GameOverMenu::update(sf::Time, sf::RenderWindow& window){
    // currently no animations, so dt is unused
    for(size_t i = 0; i < buttons.size(); i++){
        buttons[i].update(window);

        if(buttons[i].wasClicked()){
            if(i == 0){
                action = MenuAction::START_GAME;
            }else if(i == 1){
                action = MenuAction::MAIN_MENU;
            }else if(i == 2){
                action = MenuAction::QUIT;
            }
            buttons[i].reset();
        }
    }
}

void GameOverMenu::render(sf::RenderWindow& window){
    window.draw(mask);

    for(auto& button : buttons){
        button.render(window);
    }
    // NOTE: window.display() is called by Game::run(), not here
}