#pragma once
#include "Menu.h"

// Minimal options menu stub; legacy SFML UI removed
class OptionsMenu : public Menu {
public:
    explicit OptionsMenu(bool isOverlay = false);

    void update() override;
    void render() override;
    bool isOverlay() const override { return overlay; }

private:
    bool overlay = false;
};
