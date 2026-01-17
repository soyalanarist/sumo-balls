#include "PauseMenu.h"

PauseMenu::PauseMenu(sf::Font& f){
    buttons.emplace_back(f, "Resume", sf::Vector2f(300.f, 60.f), sf::Vector2f(450.f, 300.f));
    buttons.emplace_back(f, "Main Menu", sf::Vector2f(300.f, 60.f), sf::Vector2f(450.f, 380.f));
    buttons.emplace_back(f, "Options", sf::Vector2f(300.f, 70.f), sf::Vector2f(450.f, 460.f));
    buttons.emplace_back(f, "Quit", sf::Vector2f(300.f, 60.f), sf::Vector2f(450.f, 540.f));

    // Semi-transparent overlay
    mask.setSize({1200.f, 900.f});
    mask.setFillColor(sf::Color(0, 0, 0, 150));
}

void PauseMenu::update(sf::Time /*deltaTime*/, sf::RenderWindow& window){
    // currently no animations, so dt is unused
    for(auto& button : buttons){
        button.update(window);
    }

    if(buttons[0].wasClicked()){
        action = MenuAction::RESUME;
        buttons[0].reset();
    }else if(buttons[1].wasClicked()){
        action = MenuAction::MAIN_MENU;
        buttons[1].reset();
    }else if(buttons[2].wasClicked()){
        action = MenuAction::OPTIONS;
        buttons[2].reset();
    }else if(buttons[3].wasClicked()){
        action = MenuAction::QUIT;
        buttons[3].reset();
    }
}

void PauseMenu::render(sf::RenderWindow& window) {
    window.draw(mask);

    for(auto& button : buttons){
        button.render(window);
    }
    // NOTE: window.display() is called by Game::run(), not here
}