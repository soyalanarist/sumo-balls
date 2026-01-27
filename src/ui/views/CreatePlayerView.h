#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include <string>
#include <cstdint>

class CreatePlayerView : public Screen {
public:
    CreatePlayerView();

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }

private:
    ScreenTransition action = ScreenTransition::NONE;
    
    // UI state
    char handleInput[21] = "";  // Max 20 chars + null terminator
    char taglineInput[5] = "";  // Exactly 4 chars + null terminator
    std::string errorMessage;
    std::string statusMessage;
    bool isChecking = false;
    bool isAvailable = false;
    bool isSubmitting = false;
    uint32_t lastCheckTime = 0;
    std::string pendingCheck;
    
    void checkAvailability(const std::string& handle);
    void submitHandle();
    void validateInput();
};
