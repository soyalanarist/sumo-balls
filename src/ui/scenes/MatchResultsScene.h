#pragma once
#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include <string>

// Minimal overlay for match end
class MatchResultsScene : public Screen {
public:
    MatchResultsScene();
    
    void update() override;
    void render() override;
    bool isOverlay() const override { return true; }
    
    ScreenTransition getTransition() const override;
    void resetTransition() override;

private:
    std::string winnerText;
    bool returnToMenu = false;
};
