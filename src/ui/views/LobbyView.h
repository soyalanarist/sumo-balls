#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include "network/SocialManager.h"
#include <string>
#include <vector>

// Lobby browser and join screen
class LobbyView : public Screen {
public:
    LobbyView();
    ~LobbyView() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    ScreenTransition getTransition() const override;
    void resetTransition() override;

private:
    ScreenTransition action = ScreenTransition::NONE;
    std::string statusMessage;
    
    std::vector<Lobby> availableLobbies;
    bool loading = false;
    bool showCreateLobbyModal = false;
    int createMaxPlayers = 4;
    
    void loadLobbies();
    void createLobby();
    void joinLobby(const std::string& lobbyId);
};
