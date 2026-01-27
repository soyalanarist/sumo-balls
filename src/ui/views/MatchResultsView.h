#ifndef GAME_OVER_MENU_SCREEN_H
#define GAME_OVER_MENU_SCREEN_H

#include "core/Screen.h"
#include "core/ScreenTransition.h"

class MatchResultsView : public Screen {
public:
    MatchResultsView();
    ~MatchResultsView() override = default;
    
    void update() override;
    void render() override;
    bool handleInput(const SDL_Event&) override { return false; }
    
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }
    bool isOverlay() const override { return false; }
    
private:
    ScreenTransition action = ScreenTransition::NONE;
};

#endif // GAME_OVER_MENU_SCREEN_H
