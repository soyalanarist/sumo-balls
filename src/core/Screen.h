#pragma once
#include <SFML/Graphics.hpp>

// Forward declare to avoid circular includes
enum class MenuAction;

class Screen {
public:
    virtual ~Screen() = default;

    // ! replace all float dt's with sf::Time deltaTime as seen here
    virtual void update(sf::Time /*deltaTime*/, sf::RenderWindow& window) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    virtual bool isOverlay() const { return false; }
    
    // Menu action interface - screens that support menus override this
    virtual MenuAction getMenuAction() const;
    virtual void resetMenuAction() {}
};