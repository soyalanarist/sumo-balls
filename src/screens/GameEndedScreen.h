#pragma once
#include "../core/Screen.h"
#include <string>

// Minimal overlay for match end; SFML dependencies removed
class GameEndedScreen : public Screen {
public:
    GameEndedScreen();
    
    void update() override;
    void render() override;
    bool isOverlay() const override { return true; }
    
    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

private:
    std::string winnerText;
    bool returnToMenu = false;
};
