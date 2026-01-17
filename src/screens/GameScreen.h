#pragma once
#include "../core/Screen.h"
#include "../game/entities/PlayerEntity.h"
#include "../game/entities/Arena.h"
#include <vector>

class GameScreen : public Screen {
public:
    GameScreen();

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    void resolvePlayerCollisions();

private:
    Arena arena;
    std::vector<PlayerEntity> players;
    std::vector<sf::Vector2f> positions;  // Cache for player positions
    int frameCount = 0;  // Instance member instead of static
};
