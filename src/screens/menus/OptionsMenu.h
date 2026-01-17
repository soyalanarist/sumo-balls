#pragma once
#include "Menu.h"
#include "../../ui/Button.h"
#include "../../ui/ToggleSwitch.h"
#include <memory>

class OptionsMenu : public Menu {
public:
    explicit OptionsMenu(sf::Font& f);

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

private:
    std::vector<Button> buttons;
    std::unique_ptr<ToggleSwitch> leftyModeSwitch;
    sf::Text titleText;
    sf::Text statusText;  // Shows current lefty mode status
    sf::Text labelText;   // Label for the toggle switch
};
