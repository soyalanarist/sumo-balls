#include "FriendsScreen.h"
#include "../core/Settings.h"
#include "../network/HttpClient.h"
#include <cmath>

FriendsScreen::FriendsScreen(sf::Font& f)
    : font(f), action(MenuAction::NONE), currentState(State::LOADING) {
    loadFriends();
}

FriendsScreen::~FriendsScreen() = default;

void FriendsScreen::update(sf::Time deltaTime, sf::RenderWindow& window) {
    float dt = deltaTime.asSeconds();
    loadingSpinner += dt;
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
            } else if (event.key.code == sf::Keyboard::Return && currentState == State::ADD_FRIEND && searchInputActive) {
                sendFriendRequest();
            }
        } else if (event.type == sf::Event::TextEntered && searchInputActive && currentState == State::ADD_FRIEND) {
            if (event.text.unicode == 8 && !searchUsername.empty()) {
                searchUsername.pop_back();
            } else if (event.text.unicode >= 32 && event.text.unicode < 127 && searchUsername.length() < 30) {
                searchUsername += static_cast<char>(event.text.unicode);
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            // Back button
            if (mousePos.x >= 20 && mousePos.x <= 100 && mousePos.y >= 20 && mousePos.y <= 60) {
                action = MenuAction::MAIN_MENU;
            }
            // Add Friend button
            else if (mousePos.x >= 220 && mousePos.x <= 340 && mousePos.y >= 20 && mousePos.y <= 60) {
                currentState = State::ADD_FRIEND;
                searchInputActive = true;
            }
            // Pending Requests button
            else if (mousePos.x >= 470 && mousePos.x <= 600 && mousePos.y >= 20 && mousePos.y <= 60) {
                loadPendingRequests();
            }
            // In add friend state - search box click
            else if (currentState == State::ADD_FRIEND && mousePos.x >= 100 && mousePos.x <= 500 && mousePos.y >= 150 && mousePos.y <= 190) {
                searchInputActive = true;
            }
            // Send request button
            else if (currentState == State::ADD_FRIEND && mousePos.x >= 100 && mousePos.x <= 220 && mousePos.y >= 220 && mousePos.y <= 260) {
                sendFriendRequest();
            }
            // Friend remove buttons
            else if (currentState == State::FRIENDS_LIST) {
                float y = 130;
                for (size_t i = 0; i < friends.size(); ++i) {
                    if (y > 650) break;
                    if (mousePos.x >= 680 && mousePos.x <= 750 && mousePos.y >= y + 8 && mousePos.y <= y + 43) {
                        removeFriend(friends[i].userId);
                        break;
                    }
                    y += 55;
                }
            }
            // Pending requests accept/reject
            else if (currentState == State::PENDING_REQUESTS) {
                float y = 130;
                for (size_t i = 0; i < pendingRequests.size(); ++i) {
                    if (y > 650) break;
                    // Accept button
                    if (mousePos.x >= 600 && mousePos.x <= 670 && mousePos.y >= y + 8 && mousePos.y <= y + 43) {
                        acceptFriendRequest(pendingRequests[i].userId);
                        break;
                    }
                    // Reject button
                    else if (mousePos.x >= 680 && mousePos.x <= 750 && mousePos.y >= y + 8 && mousePos.y <= y + 43) {
                        removeFriend(pendingRequests[i].userId);
                        break;
                    }
                    y += 55;
                }
            }
        }
    }
}

void FriendsScreen::render(sf::RenderWindow& window) {
    window.clear(sf::Color(20, 20, 20));

    // Top bar with buttons
    if (currentState != State::LOADING) {
        // Back button area
        sf::RectangleShape backBtn({80, 40});
        backBtn.setPosition({20, 20});
        backBtn.setFillColor(sf::Color(50, 50, 50));
        window.draw(backBtn);
        
        sf::Text backText("Back", font, 14);
        backText.setPosition({35, 25});
        backText.setFillColor(sf::Color::White);
        window.draw(backText);

        // Add Friend button
        sf::RectangleShape addBtn({120, 40});
        addBtn.setPosition({220, 20});
        addBtn.setFillColor(currentState == State::ADD_FRIEND ? sf::Color(50, 100, 50) : sf::Color(50, 50, 50));
        window.draw(addBtn);
        
        sf::Text addText("Add Friend", font, 14);
        addText.setPosition({230, 25});
        addText.setFillColor(sf::Color::White);
        window.draw(addText);

        // Pending Requests button
        sf::RectangleShape pendingBtn({130, 40});
        pendingBtn.setPosition({470, 20});
        pendingBtn.setFillColor(currentState == State::PENDING_REQUESTS ? sf::Color(100, 100, 50) : sf::Color(50, 50, 50));
        window.draw(pendingBtn);
        
        sf::Text pendingText("Pending (" + std::to_string(pendingRequests.size()) + ")", font, 12);
        pendingText.setPosition({480, 28});
        pendingText.setFillColor(sf::Color::White);
        window.draw(pendingText);
    }

    // Render current state
    if (currentState == State::LOADING) {
        sf::Text loadingText("Loading Friends...", font, 24);
        loadingText.setPosition({300, 300});
        loadingText.setFillColor(sf::Color::White);
        window.draw(loadingText);
    } else if (currentState == State::FRIENDS_LIST) {
        renderFriendsList(window);
    } else if (currentState == State::ADD_FRIEND) {
        renderAddFriend(window);
    } else if (currentState == State::PENDING_REQUESTS) {
        renderPendingRequests(window);
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

void FriendsScreen::renderFriendsList(sf::RenderWindow& window) {
    sf::Text titleText("Friends (" + std::to_string(friends.size()) + ")", font, 20);
    titleText.setPosition({50, 80});
    titleText.setFillColor(sf::Color::White);
    window.draw(titleText);

    if (friends.empty()) {
        sf::Text emptyText("No friends yet. Add one!", font, 16);
        emptyText.setPosition({150, 200});
        emptyText.setFillColor(sf::Color(128, 128, 128));
        window.draw(emptyText);
        return;
    }

    float y = 130;
    for (size_t i = 0; i < friends.size(); ++i) {
        if (y > 650) break;

        const auto& frnd = friends[i];
        
        // Friend item background
        sf::RectangleShape itemBg({700, 50});
        itemBg.setPosition({50, y});
        itemBg.setFillColor(sf::Color(40, 40, 40));
        itemBg.setOutlineThickness(1);
        itemBg.setOutlineColor(sf::Color(80, 80, 80));
        window.draw(itemBg);

        // Online indicator
        sf::CircleShape onlineDot(4);
        onlineDot.setPosition({65, y + 21});
        onlineDot.setFillColor(frnd.isOnline ? sf::Color::Green : sf::Color::Red);
        window.draw(onlineDot);

        // Username
        sf::Text usernameText(frnd.username, font, 16);
        usernameText.setPosition({80, y + 10});
        usernameText.setFillColor(sf::Color::White);
        window.draw(usernameText);

        // Status
        sf::Text statusText(frnd.isOnline ? "Online" : "Offline", font, 12);
        statusText.setPosition({500, y + 15});
        statusText.setFillColor(frnd.isOnline ? sf::Color::Green : sf::Color(128, 128, 128));
        window.draw(statusText);

        // Remove button
        sf::RectangleShape removeBtn({70, 35});
        removeBtn.setPosition({680, y + 8});
        removeBtn.setFillColor(sf::Color(150, 50, 50));
        window.draw(removeBtn);
        
        sf::Text removeText("Remove", font, 12);
        removeText.setPosition({690, y + 10});
        removeText.setFillColor(sf::Color::White);
        window.draw(removeText);

        y += 55;
    }
}

void FriendsScreen::renderAddFriend(sf::RenderWindow& window) {
    sf::Text titleText("Add Friend", font, 20);
    titleText.setPosition({50, 80});
    titleText.setFillColor(sf::Color::White);
    window.draw(titleText);

    // Search box
    sf::RectangleShape searchBox({400, 40});
    searchBox.setPosition({100, 150});
    searchBox.setFillColor(searchInputActive ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
    searchBox.setOutlineThickness(2);
    searchBox.setOutlineColor(searchInputActive ? sf::Color(100, 150, 100) : sf::Color(80, 80, 80));
    window.draw(searchBox);

    sf::Text inputText(searchUsername + (searchInputActive ? "|" : ""), font, 16);
    inputText.setPosition({115, 158});
    inputText.setFillColor(sf::Color::White);
    window.draw(inputText);

    // Send request button
    sf::RectangleShape sendBtn({120, 40});
    sendBtn.setPosition({100, 220});
    sendBtn.setFillColor(sf::Color(50, 100, 50));
    window.draw(sendBtn);
    
    sf::Text sendText("Send Request", font, 14);
    sendText.setPosition({105, 225});
    sendText.setFillColor(sf::Color::White);
    window.draw(sendText);

    // Instructions
    sf::Text instrText("Type username and press Send or Enter", font, 12);
    instrText.setPosition({100, 290});
    instrText.setFillColor(sf::Color(128, 128, 128));
    window.draw(instrText);
}

void FriendsScreen::renderPendingRequests(sf::RenderWindow& window) {
    sf::Text titleText("Pending Requests (" + std::to_string(pendingRequests.size()) + ")", font, 20);
    titleText.setPosition({50, 80});
    titleText.setFillColor(sf::Color::White);
    window.draw(titleText);

    if (pendingRequests.empty()) {
        sf::Text emptyText("No pending requests", font, 16);
        emptyText.setPosition({150, 200});
        emptyText.setFillColor(sf::Color(128, 128, 128));
        window.draw(emptyText);
        return;
    }

    float y = 130;
    for (size_t i = 0; i < pendingRequests.size(); ++i) {
        if (y > 650) break;

        const auto& req = pendingRequests[i];

        sf::RectangleShape itemBg({700, 50});
        itemBg.setPosition({50, y});
        itemBg.setFillColor(sf::Color(40, 40, 40));
        itemBg.setOutlineThickness(1);
        itemBg.setOutlineColor(sf::Color(80, 80, 80));
        window.draw(itemBg);

        sf::Text usernameText(req.username, font, 16);
        usernameText.setPosition({80, y + 10});
        usernameText.setFillColor(sf::Color::White);
        window.draw(usernameText);

        // Accept button
        sf::RectangleShape acceptBtn({70, 35});
        acceptBtn.setPosition({600, y + 8});
        acceptBtn.setFillColor(sf::Color(50, 100, 50));
        window.draw(acceptBtn);
        
        sf::Text acceptText("Accept", font, 12);
        acceptText.setPosition({610, y + 10});
        acceptText.setFillColor(sf::Color::White);
        window.draw(acceptText);

        // Reject button
        sf::RectangleShape rejectBtn({70, 35});
        rejectBtn.setPosition({680, y + 8});
        rejectBtn.setFillColor(sf::Color(150, 50, 50));
        window.draw(rejectBtn);
        
        sf::Text rejectText("Reject", font, 12);
        rejectText.setPosition({690, y + 10});
        rejectText.setFillColor(sf::Color::White);
        window.draw(rejectText);

        y += 55;
    }
}

void FriendsScreen::loadFriends() {
    currentState = State::LOADING;
    friends.clear();
    
    std::string response = HttpClient::get("http://localhost:8888/friends/list", Settings::authToken);
    
    // Simple JSON parsing
    size_t pos = 0;
    while ((pos = response.find("{", pos)) != std::string::npos) {
        Friend f;
        
        // Extract userId
        size_t idPos = response.find("\"userId\":", pos);
        if (idPos != std::string::npos) {
            f.userId = std::stoi(response.substr(idPos + 9, 10));
        }
        
        // Extract username
        size_t userPos = response.find("\"username\":\"", pos);
        if (userPos != std::string::npos) {
            size_t endPos = response.find("\"", userPos + 12);
            f.username = response.substr(userPos + 12, endPos - (userPos + 12));
        }
        
        // Extract isOnline
        size_t onlinePos = response.find("\"isOnline\":", pos);
        if (onlinePos != std::string::npos) {
            f.isOnline = response.substr(onlinePos + 11, 4) == "true";
        }
        
        friends.push_back(f);
        pos++;
    }
    
    currentState = State::FRIENDS_LIST;
    setStatusMessage("Loaded " + std::to_string(friends.size()) + " friends", 2.0f);
}

void FriendsScreen::loadPendingRequests() {
    currentState = State::LOADING;
    pendingRequests.clear();
    
    std::string response = HttpClient::get("http://localhost:8888/friends/pending", Settings::authToken);
    
    size_t pos = 0;
    while ((pos = response.find("{", pos)) != std::string::npos) {
        Friend f;
        
        size_t idPos = response.find("\"userId\":", pos);
        if (idPos != std::string::npos) {
            f.userId = std::stoi(response.substr(idPos + 9, 10));
        }
        
        size_t userPos = response.find("\"username\":\"", pos);
        if (userPos != std::string::npos) {
            size_t endPos = response.find("\"", userPos + 12);
            f.username = response.substr(userPos + 12, endPos - (userPos + 12));
        }
        
        pendingRequests.push_back(f);
        pos++;
    }
    
    currentState = State::PENDING_REQUESTS;
}

void FriendsScreen::sendFriendRequest() {
    if (searchUsername.empty()) {
        setStatusMessage("Enter a username first", 2.0f);
        return;
    }
    
    setStatusMessage("Sending request...", 3.0f);
    
    std::string body = "{\"username\":\"" + searchUsername + "\"}";
    std::string response = HttpClient::post("http://localhost:8888/friends/send-request", body, Settings::authToken);
    
    if (response.find("\"success\":true") != std::string::npos) {
        setStatusMessage("Request sent!", 2.0f);
        searchUsername.clear();
        currentState = State::FRIENDS_LIST;
    } else {
        setStatusMessage("Failed to send request", 2.0f);
    }
}

void FriendsScreen::acceptFriendRequest(int userId) {
    setStatusMessage("Accepting...", 3.0f);
    
    std::string body = "{\"fromUserId\":" + std::to_string(userId) + "}";
    HttpClient::post("http://localhost:8888/friends/accept-request", body, Settings::authToken);
    
    loadPendingRequests();
    loadFriends();
}

void FriendsScreen::removeFriend(int userId) {
    setStatusMessage("Removing friend...", 3.0f);
    
    std::string body = "{\"friendId\":" + std::to_string(userId) + "}";
    HttpClient::post("http://localhost:8888/friends/remove", body, Settings::authToken);
    
    loadFriends();
}

void FriendsScreen::setStatusMessage(const std::string& msg, float duration) {
    statusMessage = msg;
    statusTimer = duration;
}
