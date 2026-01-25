#pragma once

#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include <string>

// Minimal lobby screen stub; legacy SFML UI removed
class LobbyScreen : public Screen {
public:
    LobbyScreen();
    ~LobbyScreen() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

private:
    MenuAction action = MenuAction::NONE;
    std::string statusMessage;
};
