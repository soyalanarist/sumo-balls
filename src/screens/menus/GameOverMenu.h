#include "Menu.h"
#include "../../ui/Button.h"

class GameOverMenu : public Menu {
public:
    explicit GameOverMenu(sf::Font& f);

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

    bool isOverlay() const override { return true; }

private:
    std::vector<Button> buttons;
    sf::RectangleShape mask;
};