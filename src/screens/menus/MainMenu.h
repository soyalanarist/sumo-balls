#include "Menu.h"
#include "../../ui/Button.h"

class MainMenu : public Menu {
public:
    explicit MainMenu(sf::Font& f);

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;

private:
    std::vector<Button> buttons;
};
