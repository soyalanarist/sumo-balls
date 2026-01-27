#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include <string>
#include <cstdint>

class LoginView : public Screen {
public:
    LoginView();

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }

private:
    ScreenTransition action = ScreenTransition::NONE;
    
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

    // Test automation state
    bool autoLoginAttempted = false;
    bool disableGoogleAuth = false;
    void attemptCredentialLogin();
    
    void attemptGoogleLogin();
    void pollGoogleStatus();
};
