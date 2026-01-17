#pragma once

#include "../../core/Screen.h"
#include "MenuAction.h"

class Menu : public Screen {
public:
    // Legacy interface for compatibility
    MenuAction getAction() const { return action; }
    void resetAction() { action = MenuAction::NONE; }
    
    // New virtual interface from Screen
    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }
    bool isOverlay() const override { return false; }  // Menus are not overlays

protected:
    MenuAction action = MenuAction::NONE;
};
