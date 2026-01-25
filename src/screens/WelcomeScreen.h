#pragma once

#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include <string>
#include <cstdint>

class WelcomeScreen : public Screen {
public:
    WelcomeScreen();

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    MenuAction action = MenuAction::NONE;
    
    // UI state
    std::string errorMessage;
    std::string statusMessage;
    bool isLoading = false;
    
    // Google OAuth polling state
    bool googleOAuthInProgress = false;
    std::string googleSessionId;
    uint32_t googleOAuthStartTime = 0;
    uint32_t googleLastPollTime = 0;
    const uint32_t googleOAuthTimeout = 30000; // 30 seconds in ms
    const uint32_t googlePollInterval = 500;    // Poll every 500ms
    
    void attemptGoogleLogin();
    void pollGoogleStatus();
};
