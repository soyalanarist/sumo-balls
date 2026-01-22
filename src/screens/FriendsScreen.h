#ifndef FRIENDS_SCREEN_H
#define FRIENDS_SCREEN_H

#include <SFML/Graphics.hpp>
#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include <vector>
#include <string>

struct Friend {
    int userId;
    std::string username;
    bool isOnline;
};

class FriendsScreen : public Screen {
public:
    explicit FriendsScreen(sf::Font& f);
    ~FriendsScreen();

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    enum State {
        LOADING,
        FRIENDS_LIST,
        ADD_FRIEND,
        PENDING_REQUESTS
    };

    sf::Font& font;
    MenuAction action = MenuAction::NONE;
    State currentState = State::LOADING;
    
    std::vector<Friend> friends;
    std::vector<Friend> pendingRequests;
    std::string searchUsername;
    std::string statusMessage;
    float statusTimer = 0;
    float loadingSpinner = 0;
    float scrollOffset = 0;
    bool searchInputActive = false;
    
    // Helper methods
    void loadFriends();
    void loadPendingRequests();
    void sendFriendRequest();
    void acceptFriendRequest(int userId);
    void removeFriend(int userId);
    void renderFriendsList(sf::RenderWindow& window);
    void renderAddFriend(sf::RenderWindow& window);
    void renderPendingRequests(sf::RenderWindow& window);
    void setStatusMessage(const std::string& msg, float duration = 3.0f);
};

#endif
