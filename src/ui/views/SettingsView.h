#ifndef OPTIONS_MENU_SCREEN_H
#define OPTIONS_MENU_SCREEN_H

#include "core/Screen.h"
#include "core/ScreenTransition.h"

class SettingsView : public Screen {
public:
    explicit SettingsView(bool isOverlay = false);
    ~SettingsView() override = default;
    
    void update() override;
    void render() override;
    bool handleInput(const SDL_Event&) override { return false; }
    
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }
    bool isOverlay() const override { return overlay; }
    
private:
    ScreenTransition action = ScreenTransition::NONE;
    bool overlay = false;
    bool initialized = false;
    
    int pendingDisplayMode = 0;
    bool pendingLeftHanded = false;
    int pendingAIDifficulty = 0;
    
    float appliedTimer = 0.0f;
};

#endif // OPTIONS_MENU_SCREEN_H
