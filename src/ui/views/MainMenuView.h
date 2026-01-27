#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"

class MainMenuView : public Screen {
public:
    MainMenuView();

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }

private:
    ScreenTransition action = ScreenTransition::NONE;
};
