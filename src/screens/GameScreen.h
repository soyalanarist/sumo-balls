#pragma once
#include "../core/Screen.h"
#include "../game/entities/PlayerEntity.h"
#include "../game/entities/Arena.h"
#include "menus/MenuAction.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;        // Total lifetime in seconds
    float timeRemaining;   // Time left before particle dies
    float radius;
};

class GameScreen : public Screen {
public:
    GameScreen();

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    void resolvePlayerCollisions();
    void createExplosion(sf::Vector2f position, sf::Vector2f velocity);
    void updateParticles(float dt);
    
    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

private:
    Arena arena;
    std::vector<PlayerEntity> players;
    std::vector<sf::Vector2f> positions;  // Cache for player positions
    std::vector<Particle> particles;      // Explosion particles
    int frameCount = 0;  // Instance member instead of static
    bool gameOver = false;
    float gameOverTime = 0.f;             // Time since game ended
    MenuAction menuAction = MenuAction::NONE;  // Track requested menu action
};
