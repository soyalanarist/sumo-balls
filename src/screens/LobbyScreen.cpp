#include "LobbyScreen.h"
#include "../core/Settings.h"
#include "../network/HttpClient.h"

LobbyScreen::LobbyScreen(sf::Font& f, const std::string& id)
    : font(f), lobbyId(id) {
    if (!lobbyId.empty()) {
        currentState = State::LOADING;
        loadLobbies();
    } else {
        loadLobbies();
    }
}

LobbyScreen::~LobbyScreen() = default;

void LobbyScreen::update(sf::Time deltaTime, sf::RenderWindow& window) {
    float dt = deltaTime.asSeconds();
    if (statusTimer > 0) statusTimer -= dt;

    // Handle input
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            return;
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                action = MenuAction::MAIN_MENU;
            } else if (event.key.code == sf::Keyboard::Return && currentState == State::CREATING && inputActive) {
                createLobby();
            }
        } else if (event.type == sf::Event::TextEntered && inputActive && currentState == State::CREATING) {
            if (event.text.unicode == 8 && !newLobbyName.empty()) {
                newLobbyName.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode < 127 && newLobbyName.length() < 30) {
                newLobbyName += static_cast<char>(event.text.unicode);
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // Back button
            if (mousePos.x >= 20 && mousePos.x <= 100 && mousePos.y >= 5 && mousePos.y <= 45) {
                action = MenuAction::MAIN_MENU;
            }
            // Browse state
            else if (currentState == State::BROWSING) {
                // Create lobby button
                if (mousePos.x >= 620 && mousePos.x <= 770 && mousePos.y >= 70 && mousePos.y <= 110) {
                    currentState = State::CREATING;
                    inputActive = true;
                }
                // Join lobby buttons
                else {
                    float y = 130;
                    for (size_t i = 0; i < availableLobby.size(); ++i) {
                        if (y > 600) break;
                        if (mousePos.x >= 650 && mousePos.x <= 730 && mousePos.y >= y + 13 && mousePos.y <= y + 48) {
                            joinLobby(availableLobby[i].id);
                            break;
                        }
                        y += 70;
                    }
                }
            }
            // Create lobby state
            else if (currentState == State::CREATING) {
                // Name input box
                if (mousePos.x >= 100 && mousePos.x <= 500 && mousePos.y >= 180 && mousePos.y <= 220) {
                    inputActive = true;
                }
                // Create button
                else if (mousePos.x >= 100 && mousePos.x <= 220 && mousePos.y >= 350 && mousePos.y <= 390) {
                    createLobby();
                }
                // Cancel button
                else if (mousePos.x >= 240 && mousePos.x <= 360 && mousePos.y >= 350 && mousePos.y <= 390) {
                    currentState = State::BROWSING;
                    newLobbyName.clear();
                    inputActive = false;
                }
            }
            // In lobby state
            else if (currentState == State::IN_LOBBY) {
                // Mark ready button
                if (mousePos.x >= 100 && mousePos.x <= 220 && mousePos.y >= 550 && mousePos.y <= 590) {
                    setReadyStatus(true);
                }
                // Leave button
                else if (mousePos.x >= 240 && mousePos.x <= 360 && mousePos.y >= 550 && mousePos.y <= 590) {
                    leaveLobby();
                }
                // Start game button (host only)
                else if (currentLobby.hostId == Settings::userID && mousePos.x >= 500 && mousePos.x <= 650 && mousePos.y >= 550 && mousePos.y <= 590) {
                    // TODO: Emit event to start game
                }
            }
        }
    }
}

void LobbyScreen::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 20, 20));

    // Top navigation bar
    sf::RectangleShape topBar({800, 50});
    topBar.setPosition({0, 0});
    topBar.setFillColor(sf::Color(30, 30, 30));
    window.draw(topBar);

    // Back button
    sf::RectangleShape backBtn({80, 40});
    backBtn.setPosition({20, 5});
    backBtn.setFillColor(sf::Color(50, 50, 50));
    window.draw(backBtn);
    
    sf::Text backText("Back", font, 14);
    backText.setPosition({35, 10});
    backText.setFillColor(sf::Color::White);
    window.draw(backText);

    // Title
    sf::Text titleText("Lobbies", font, 20);
    titleText.setPosition({350, 10});
    titleText.setFillColor(sf::Color::White);
    window.draw(titleText);

    // Main content area
    if (currentState == State::LOADING) {
        sf::Text loadingText("Loading lobbies...", font, 24);
        loadingText.setPosition({300, 300});
        loadingText.setFillColor(sf::Color::White);
        window.draw(loadingText);
    } else if (currentState == State::BROWSING) {
        renderBrowseLobbies(window);
    } else if (currentState == State::CREATING) {
        renderCreateLobby(window);
    } else if (currentState == State::IN_LOBBY) {
        renderInLobby(window);
    }

    // Status message
    if (statusTimer > 0) {
        sf::RectangleShape msgBg({400, 40});
        msgBg.setPosition({200, 650});
        msgBg.setFillColor(sf::Color(40, 40, 80));
        window.draw(msgBg);
        
        sf::Text msgText(statusMessage, font, 14);
        msgText.setPosition({210, 658});
        msgText.setFillColor(sf::Color::Yellow);
        window.draw(msgText);
    }
}

void LobbyScreen::renderBrowseLobbies(sf::RenderWindow& window) {
    // Create lobby button
    sf::RectangleShape createBtn({150, 40});
    createBtn.setPosition({620, 70});
    createBtn.setFillColor(sf::Color(50, 100, 50));
    window.draw(createBtn);
    
    sf::Text createText("Create Lobby", font, 14);
    createText.setPosition({630, 75});
    createText.setFillColor(sf::Color::White);
    window.draw(createText);

    // Available lobbies list
    sf::Text availText("Available Lobbies", font, 18);
    availText.setPosition({50, 70});
    availText.setFillColor(sf::Color::White);
    window.draw(availText);

    if (availableLobby.empty()) {
        sf::Text emptyText("No lobbies available. Create one!", font, 16);
        emptyText.setPosition({100, 200});
        emptyText.setFillColor(sf::Color(128, 128, 128));
        window.draw(emptyText);
        return;
    }

    float y = 130;
    for (const auto& lobby : availableLobby) {
        if (y > 600) break;

        sf::RectangleShape itemBg({700, 60});
        itemBg.setPosition({50, y});
        itemBg.setFillColor(sf::Color(40, 40, 40));
        itemBg.setOutlineThickness(1);
        itemBg.setOutlineColor(sf::Color(80, 80, 80));
        window.draw(itemBg);

        sf::Text nameText(lobby.name, font, 16);
        nameText.setPosition({70, y + 8});
        nameText.setFillColor(sf::Color::White);
        window.draw(nameText);

        sf::Text playersText(std::to_string(lobby.members.size()) + "/" + std::to_string(lobby.maxPlayers), font, 12);
        playersText.setPosition({400, y + 15});
        playersText.setFillColor(sf::Color(200, 200, 200));
        window.draw(playersText);

        sf::RectangleShape joinBtn({80, 35});
        joinBtn.setPosition({650, y + 13});
        joinBtn.setFillColor(sf::Color(50, 150, 50));
        window.draw(joinBtn);
        
        sf::Text joinText("Join", font, 14);
        joinText.setPosition({665, y + 15});
        joinText.setFillColor(sf::Color::White);
        window.draw(joinText);

        y += 70;
    }
}

void LobbyScreen::renderCreateLobby(sf::RenderWindow& window) {
    sf::Text titleText("Create New Lobby", font, 20);
    titleText.setPosition({50, 80});
    titleText.setFillColor(sf::Color::White);
    window.draw(titleText);

    // Lobby name input
    sf::Text labelText("Lobby Name:", font, 14);
    labelText.setPosition({100, 150});
    labelText.setFillColor(sf::Color::White);
    window.draw(labelText);

    sf::RectangleShape nameBox({400, 40});
    nameBox.setPosition({100, 180});
    nameBox.setFillColor(inputActive ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
    nameBox.setOutlineThickness(2);
    nameBox.setOutlineColor(inputActive ? sf::Color(100, 150, 100) : sf::Color(80, 80, 80));
    window.draw(nameBox);

    sf::Text inputText(newLobbyName + (inputActive ? "|" : ""), font, 16);
    inputText.setPosition({115, 188});
    inputText.setFillColor(sf::Color::White);
    window.draw(inputText);

    // Max players selection
    sf::Text maxLabel("Max Players:", font, 14);
    maxLabel.setPosition({100, 260});
    maxLabel.setFillColor(sf::Color::White);
    window.draw(maxLabel);

    sf::Text maxText(std::to_string(maxPlayersSelection), font, 16);
    maxText.setPosition({300, 260});
    maxText.setFillColor(sf::Color(200, 200, 200));
    window.draw(maxText);

    // Create button
    sf::RectangleShape createBtn({120, 40});
    createBtn.setPosition({100, 350});
    createBtn.setFillColor(sf::Color(50, 100, 50));
    window.draw(createBtn);
    
    sf::Text createText("Create", font, 14);
    createText.setPosition({120, 355});
    createText.setFillColor(sf::Color::White);
    window.draw(createText);

    // Cancel button
    sf::RectangleShape cancelBtn({120, 40});
    cancelBtn.setPosition({240, 350});
    cancelBtn.setFillColor(sf::Color(100, 50, 50));
    window.draw(cancelBtn);
    
    sf::Text cancelText("Cancel", font, 14);
    cancelText.setPosition({255, 355});
    cancelText.setFillColor(sf::Color::White);
    window.draw(cancelText);
}

void LobbyScreen::renderInLobby(sf::RenderWindow& window) {
    // Lobby info
    sf::Text lobbyText("Lobby: " + currentLobby.name, font, 18);
    lobbyText.setPosition({50, 80});
    lobbyText.setFillColor(sf::Color::White);
    window.draw(lobbyText);

    // Members
    sf::Text membersLabel("Members:", font, 14);
    membersLabel.setPosition({50, 130});
    membersLabel.setFillColor(sf::Color(200, 200, 200));
    window.draw(membersLabel);

    float y = 160;
    for (const auto& member : currentLobby.members) {
        sf::RectangleShape memberBg({400, 40});
        memberBg.setPosition({70, y});
        memberBg.setFillColor(sf::Color(40, 40, 40));
        window.draw(memberBg);

        std::string memberText = member.username + (member.isHost ? " (Host)" : "");
        sf::Text text(memberText, font, 14);
        text.setPosition({80, y + 8});
        text.setFillColor(sf::Color::White);
        window.draw(text);

        sf::Text readyText(member.isReady ? "Ready" : "Not Ready", font, 12);
        readyText.setPosition({350, y + 10});
        readyText.setFillColor(member.isReady ? sf::Color::Green : sf::Color::Red);
        window.draw(readyText);

        y += 50;
    }

    // Ready button
    sf::RectangleShape readyBtn({120, 40});
    readyBtn.setPosition({100, 550});
    readyBtn.setFillColor(sf::Color(50, 100, 50));
    window.draw(readyBtn);
    
    sf::Text readyText("Mark Ready", font, 14);
    readyText.setPosition({105, 555});
    readyText.setFillColor(sf::Color::White);
    window.draw(readyText);

    // Leave button
    sf::RectangleShape leaveBtn({120, 40});
    leaveBtn.setPosition({240, 550});
    leaveBtn.setFillColor(sf::Color(150, 50, 50));
    window.draw(leaveBtn);
    
    sf::Text leaveText("Leave", font, 14);
    leaveText.setPosition({260, 555});
    leaveText.setFillColor(sf::Color::White);
    window.draw(leaveText);

    // Start game button (only for host)
    bool isHost = currentLobby.hostId == Settings::userID;
    if (isHost) {
        sf::RectangleShape startBtn({150, 40});
        startBtn.setPosition({500, 550});
        startBtn.setFillColor(sf::Color(100, 150, 50));
        window.draw(startBtn);
        
        sf::Text startText("Start Game", font, 14);
        startText.setPosition({510, 555});
        startText.setFillColor(sf::Color::White);
        window.draw(startText);
    }
}

void LobbyScreen::loadLobbies() {
    currentState = State::LOADING;
    availableLobby.clear();
    
    std::string response = HttpClient::get("http://localhost:8888/lobby/list", Settings::authToken);
    
    // Simple JSON parsing
    size_t pos = 0;
    while ((pos = response.find("{", pos)) != std::string::npos) {
        LobbyInfo lobby;
        
        // Extract id
        size_t idPos = response.find("\"id\":\"", pos);
        if (idPos != std::string::npos) {
            size_t endPos = response.find("\"", idPos + 6);
            lobby.id = response.substr(idPos + 6, endPos - (idPos + 6));
        }
        
        // Extract name
        size_t namePos = response.find("\"name\":\"", pos);
        if (namePos != std::string::npos) {
            size_t endPos = response.find("\"", namePos + 8);
            lobby.name = response.substr(namePos + 8, endPos - (namePos + 8));
        }
        
        // Extract maxPlayers
        size_t maxPos = response.find("\"maxPlayers\":", pos);
        if (maxPos != std::string::npos) {
            lobby.maxPlayers = std::stoi(response.substr(maxPos + 13, 2));
        }
        
        availableLobby.push_back(lobby);
        pos++;
    }
    
    currentState = State::BROWSING;
    setStatusMessage("Loaded " + std::to_string(availableLobby.size()) + " lobbies", 2.0f);
}

void LobbyScreen::createLobby() {
    if (newLobbyName.empty()) {
        setStatusMessage("Enter a lobby name", 2.0f);
        return;
    }
    
    setStatusMessage("Creating lobby...", 3.0f);
    
    std::string body = "{\"name\":\"" + newLobbyName + "\",\"maxPlayers\":" + std::to_string(maxPlayersSelection) + "}";
    std::string response = HttpClient::post("http://localhost:8888/lobby/create", body, Settings::authToken);
    
    if (response.find("\"success\":true") != std::string::npos) {
        setStatusMessage("Lobby created!", 2.0f);
        newLobbyName.clear();
        loadLobbies();
    } else {
        setStatusMessage("Failed to create lobby", 2.0f);
    }
}

void LobbyScreen::joinLobby(const std::string& id) {
    setStatusMessage("Joining lobby...", 3.0f);
    
    std::string body = "{\"lobbyId\":\"" + id + "\"}";
    HttpClient::post("http://localhost:8888/lobby/join", body, Settings::authToken);
    
    currentState = State::IN_LOBBY;
    setStatusMessage("Joined lobby", 2.0f);
}

void LobbyScreen::leaveLobby() {
    setStatusMessage("Leaving lobby...", 2.0f);
    
    std::string body = "{\"lobbyId\":\"" + currentLobby.id + "\"}";
    HttpClient::post("http://localhost:8888/lobby/leave", body, Settings::authToken);
    
    action = MenuAction::MAIN_MENU;
    setStatusMessage("Left lobby", 2.0f);
}

void LobbyScreen::setReadyStatus(bool ready) {
    std::string body = "{\"lobbyId\":\"" + currentLobby.id + "\",\"isReady\":" + (ready ? "true" : "false") + "}";
    HttpClient::post("http://localhost:8888/lobby/set-ready", body, Settings::authToken);
    setStatusMessage(ready ? "Marked as ready" : "Marked as not ready", 2.0f);
}

void LobbyScreen::setStatusMessage(const std::string& msg, float duration) {
    statusMessage = msg;
    statusTimer = duration;
}
