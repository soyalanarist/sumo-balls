#include "Menu.h"
#include "../../ui/Button.h"

class PauseMenu : public Menu {
public:
    explicit PauseMenu(sf::Font& f);

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

    bool isOverlay() const override { return true; }
private:
    std::vector<Button> buttons;
    sf::RectangleShape mask;
};
