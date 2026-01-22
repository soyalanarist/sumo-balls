#pragma once

#include "../core/Screen.h"
#include "../ui/Button.h"
#include "../ui/UIElement.h"
#include "menus/MenuAction.h"
#include <SFML/Graphics.hpp>
#include <string>

class AuthScreen : public Screen {
public:
    enum class AuthMode {
        LOGIN,
        REGISTER
    };

    explicit AuthScreen(sf::Font& f);

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    sf::Font& font;
    AuthMode mode = AuthMode::LOGIN;
    std::string usernameInput;
    std::string passwordInput;
    std::string emailInput;
    
    Button loginButton;
    Button registerButton;
    Button switchModeButton;
    
    std::string statusMessage;
    float statusTimer = 0.f;
    
    MenuAction action = MenuAction::NONE;
    
    bool isPasswordFocused = false;
    bool isUsernameFocused = false;
    bool isEmailFocused = false;
    
    void attemptLogin();
    void attemptRegister();
    void switchMode();
};
