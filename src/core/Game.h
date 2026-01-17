#include "ScreenStack.h"

enum class GameState {
    MENU,
    PLAYING
};

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;
    sf::Font font;
    ScreenStack screens;
    // GameState state;
};
