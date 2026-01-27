#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"

class PauseMenuView : public Screen {
public:
    PauseMenuView();

    void update() override;
    void render() override;
    bool isOverlay() const override { return true; }
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }

private:
    ScreenTransition action = ScreenTransition::NONE;
};
