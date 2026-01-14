#include "../menus/Menu.h"

class MainMenu : public Menu {
public:
    MainMenu(sf::Font& font);
    void processInput(sf::RenderWindow& window) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    sf::Text playButton;
    sf::Text quitButton;
};