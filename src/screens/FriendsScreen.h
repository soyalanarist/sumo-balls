#pragma once

#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include <string>

// Minimal friends screen stub; SFML UI removed during SDL/ImGui migration
class FriendsScreen : public Screen {
public:
    FriendsScreen();
    ~FriendsScreen() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

    void removeFriend(const std::string &username);
    void setStatusMessage(const std::string &message);

private:
    MenuAction action = MenuAction::NONE;
    std::string statusMessage;
};
