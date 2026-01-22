#include "AuthScreen.h"
#include "../core/Settings.h"
#include "../utils/SimpleJson.h"
#include "../network/HttpClient.h"
#include <iostream>
#include <sstream>

AuthScreen::AuthScreen(sf::Font& f)
    : font(f),
      loginButton(f, "Login", sf::Vector2f(200.f, 60.f), sf::Vector2f(500.f, 350.f)),
      registerButton(f, "Register", sf::Vector2f(200.f, 60.f), sf::Vector2f(500.f, 350.f)),
      switchModeButton(f, "Switch to Register", sf::Vector2f(200.f, 50.f), sf::Vector2f(500.f, 430.f))
{
}

void AuthScreen::update(sf::Time deltaTime, sf::RenderWindow& window) {
    // Update input boxes
    sf::Event event;
    while(window.pollEvent(event)) {
        if(event.type == sf::Event::Closed) {
            window.close();
            return;
        }
        if(event.type == sf::Event::TextEntered) {
            if(isUsernameFocused && event.text.unicode < 128) {
                if(event.text.unicode == 8 && !usernameInput.empty()) {
                    usernameInput.pop_back();
                } else if(event.text.unicode >= 32 && usernameInput.length() < 20) {
                    usernameInput += static_cast<char>(event.text.unicode);
                }
            } else if(isPasswordFocused && event.text.unicode < 128) {
                if(event.text.unicode == 8 && !passwordInput.empty()) {
                    passwordInput.pop_back();
                } else if(event.text.unicode >= 32 && passwordInput.length() < 30) {
                    passwordInput += static_cast<char>(event.text.unicode);
                }
            } else if(isEmailFocused && mode == AuthMode::REGISTER && event.text.unicode < 128) {
                if(event.text.unicode == 8 && !emailInput.empty()) {
                    emailInput.pop_back();
                } else if(event.text.unicode >= 32 && emailInput.length() < 50) {
                    emailInput += static_cast<char>(event.text.unicode);
                }
            }
        }
        if(event.type == sf::Event::MouseButtonPressed) {
            // Check if username input is clicked
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            
            // Check each input box and set focus accordingly
            bool clickedUsername = (mousePos.x >= 350.f && mousePos.x <= 650.f && mousePos.y >= 200.f && mousePos.y <= 240.f);
            bool clickedPassword = (mousePos.x >= 350.f && mousePos.x <= 650.f && mousePos.y >= 260.f && mousePos.y <= 300.f);
            bool clickedEmail = mode == AuthMode::REGISTER && (mousePos.x >= 350.f && mousePos.x <= 650.f && mousePos.y >= 320.f && mousePos.y <= 360.f);
            
            isUsernameFocused = clickedUsername;
            isPasswordFocused = clickedPassword;
            isEmailFocused = clickedEmail;
        }
    }

    loginButton.update(window);
    registerButton.update(window);
    switchModeButton.update(window);

    // Update button states based on mode
    if(mode == AuthMode::LOGIN && loginButton.wasClicked()) {
        loginButton.reset();
        attemptLogin();
    } else if(mode == AuthMode::REGISTER && registerButton.wasClicked()) {
        registerButton.reset();
        attemptRegister();
    }

    if(switchModeButton.wasClicked()) {
        switchModeButton.reset();
        switchMode();
    }

    // Update status message timer
    if(statusTimer > 0.f) {
        statusTimer -= deltaTime.asSeconds();
    }
}

void AuthScreen::render(sf::RenderWindow& window) {
    // Title
    sf::Text title;
    title.setFont(font);
    title.setString(mode == AuthMode::LOGIN ? "Login" : "Register");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f, 50.f);
    window.draw(title);

    // Username input
    sf::RectangleShape usernameBox(sf::Vector2f(300.f, 40.f));
    usernameBox.setPosition(350.f, 200.f);
    usernameBox.setFillColor(isUsernameFocused ? sf::Color(0, 200, 0) : sf::Color(40, 40, 40));
    usernameBox.setOutlineThickness(2.f);
    usernameBox.setOutlineColor(isUsernameFocused ? sf::Color(0, 255, 0) : sf::Color(100, 100, 100));
    window.draw(usernameBox);

    sf::Text usernameLabel;
    usernameLabel.setFont(font);
    usernameLabel.setString("Username: " + usernameInput + (isUsernameFocused ? "|" : ""));
    usernameLabel.setCharacterSize(20);
    usernameLabel.setFillColor(sf::Color::White);
    usernameLabel.setPosition(360.f, 207.f);
    window.draw(usernameLabel);

    // Password input
    sf::RectangleShape passwordBox(sf::Vector2f(300.f, 40.f));
    passwordBox.setPosition(350.f, 260.f);
    passwordBox.setFillColor(isPasswordFocused ? sf::Color(0, 200, 0) : sf::Color(40, 40, 40));
    passwordBox.setOutlineThickness(2.f);
    passwordBox.setOutlineColor(isPasswordFocused ? sf::Color(0, 255, 0) : sf::Color(100, 100, 100));
    window.draw(passwordBox);

    std::string passwordDisplay(passwordInput.length(), '*');
    sf::Text passwordLabel;
    passwordLabel.setFont(font);
    passwordLabel.setString("Password: " + passwordDisplay + (isPasswordFocused ? "|" : ""));
    passwordLabel.setCharacterSize(20);
    passwordLabel.setFillColor(sf::Color::White);
    passwordLabel.setPosition(360.f, 267.f);
    window.draw(passwordLabel);

    // Email input (register only)
    if(mode == AuthMode::REGISTER) {
        sf::RectangleShape emailBox(sf::Vector2f(300.f, 40.f));
        emailBox.setPosition(350.f, 320.f);
        emailBox.setFillColor(isEmailFocused ? sf::Color(0, 200, 0) : sf::Color(40, 40, 40));
        emailBox.setOutlineThickness(2.f);
        emailBox.setOutlineColor(isEmailFocused ? sf::Color(0, 255, 0) : sf::Color(100, 100, 100));
        window.draw(emailBox);

        sf::Text emailLabel;
        emailLabel.setFont(font);
        emailLabel.setString("Email: " + emailInput + (isEmailFocused ? "|" : ""));
        emailLabel.setCharacterSize(20);
        emailLabel.setFillColor(sf::Color::White);
        emailLabel.setPosition(360.f, 327.f);
        window.draw(emailLabel);
    }

    // Buttons
    if(mode == AuthMode::LOGIN) {
        loginButton.render(window);
    } else {
        registerButton.render(window);
    }
    switchModeButton.render(window);

    // Status message
    if(statusTimer > 0.f) {
        sf::Text status;
        status.setFont(font);
        status.setString(statusMessage);
        status.setCharacterSize(18);
        status.setFillColor(statusMessage.find("success") != std::string::npos ? sf::Color::Green : sf::Color::Red);
        status.setPosition(350.f, 480.f);
        window.draw(status);
    }
}

void AuthScreen::attemptLogin() {
    if(usernameInput.empty() || passwordInput.empty()) {
        statusMessage = "Username and password required";
        statusTimer = 3.f;
        return;
    }

    // Build JSON request
    SimpleJson req;
    req.set("username", usernameInput);
    req.set("password", passwordInput);

    // Make HTTP request to coordinator
    std::string response = HttpClient::post("http://127.0.0.1:8888/auth/login", req.toString());
    SimpleJson resp = SimpleJson::parse(response);

    if(resp.getBool("success")) {
        Settings::authToken = resp.getString("token");
        Settings::username = usernameInput;
        // TODO: Parse user ID from response if needed
        statusMessage = "Login successful!";
        statusTimer = 1.f;
        action = MenuAction::MAIN_MENU;
    } else {
        statusMessage = resp.getString("message", "Login failed");
        statusTimer = 3.f;
    }
}

void AuthScreen::attemptRegister() {
    if(usernameInput.empty() || passwordInput.empty() || emailInput.empty()) {
        statusMessage = "All fields required";
        statusTimer = 3.f;
        return;
    }

    if(usernameInput.length() < 3) {
        statusMessage = "Username must be 3+ characters";
        statusTimer = 3.f;
        return;
    }

    if(passwordInput.length() < 6) {
        statusMessage = "Password must be 6+ characters";
        statusTimer = 3.f;
        return;
    }

    // Build JSON request
    SimpleJson req;
    req.set("username", usernameInput);
    req.set("password", passwordInput);
    req.set("email", emailInput);

    // Make HTTP request to coordinator
    std::string response = HttpClient::post("http://127.0.0.1:8888/auth/register", req.toString());
    SimpleJson resp = SimpleJson::parse(response);

    if(resp.getBool("success")) {
        Settings::authToken = resp.getString("token");
        Settings::username = usernameInput;
        statusMessage = "Registration successful!";
        statusTimer = 1.f;
        action = MenuAction::MAIN_MENU;
    } else {
        statusMessage = resp.getString("message", "Registration failed");
        statusTimer = 3.f;
    }
}

void AuthScreen::switchMode() {
    mode = (mode == AuthMode::LOGIN) ? AuthMode::REGISTER : AuthMode::LOGIN;
    usernameInput.clear();
    passwordInput.clear();
    emailInput.clear();
    statusMessage.clear();
    statusTimer = 0.f;
}
