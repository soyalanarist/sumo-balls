#pragma once
#include "Menu.h"

class PauseMenu : public Menu {
public:
    PauseMenu();

    void update() override;
    void render() override;

    bool isOverlay() const override { return true; }
};
