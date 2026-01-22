#ifndef LOBBY_SCREEN_H
#define LOBBY_SCREEN_H

#include <SFML/Graphics.hpp>
#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include <vector>
#include <string>

class LobbyScreen : public Screen {
public:
    struct LobbyMember {
        int userId;
        std::string username;
        bool isReady;
        bool isHost;
    };

    struct LobbyInfo {
        std::string id;
        std::string name;
        int hostId;
        std::vector<LobbyMember> members;
        int maxPlayers;
    };

    enum class State {
        BROWSING,
        CREATING,
        JOINING,
        IN_LOBBY,
        LOADING
    };

    explicit LobbyScreen(sf::Font& f, const std::string& lobbyId = "");
    ~LobbyScreen();

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    // Data structures
    sf::Font& font;
    MenuAction action = MenuAction::NONE;
    State currentState = State::LOADING;
    
    std::vector<LobbyInfo> availableLobby;
    LobbyInfo currentLobby;
    std::string lobbyId;
    
    // UI state
    std::string newLobbyName;
    int maxPlayersSelection = 4;
    float statusTimer = 0.0f;
    std::string statusMessage;
    float scrollOffset = 0.0f;
    bool inputActive = false;
    
    // Methods
    void loadLobbies();
    void createLobby();
    void joinLobby(const std::string& id);
    void leaveLobby();
    void setReadyStatus(bool ready);
    void refreshLobbyList();
    
    // Render methods
    void renderBrowseLobbies(sf::RenderWindow& window);
    void renderCreateLobby(sf::RenderWindow& window);
    void renderInLobby(sf::RenderWindow& window);
    
    void setStatusMessage(const std::string& msg, float duration = 2.0f);
};

#endif
