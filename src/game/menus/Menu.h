#include <SFML/Graphics.hpp>

class Menu {
public:
    virtual ~Menu() = default;
    virtual void processInput(sf::RenderWindow& window) = 0;
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;
};
