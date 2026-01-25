#pragma once
#include "Menu.h"

// Minimal game over menu stub; visuals removed during migration
class GameOverMenu : public Menu {
public:
    GameOverMenu();

    void update() override;
    void render() override;

    bool isOverlay() const override { return true; }
};