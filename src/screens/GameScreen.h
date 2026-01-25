#pragma once

#include "../core/Screen.h"
#include "menus/MenuAction.h"

// Stubbed GameScreen without SFML; gameplay removed during migration
class GameScreen : public Screen {
public:
    explicit GameScreen(bool singlePlayer = false);
    ~GameScreen() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }
    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    bool singleplayer = false;
    MenuAction action = MenuAction::NONE;
};
