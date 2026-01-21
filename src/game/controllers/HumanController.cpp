#include "HumanController.h"
#include "../../core/Settings.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <vector>

static sf::Vector2f normalize(const sf::Vector2f& v){
    float len = std::sqrt(v.x * v.x + v.y * v.y);
    if (len == 0.f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

sf::Vector2f HumanController::getMovementDirection(
    float /*dt*/,
    const sf::Vector2f& /*selfPosition*/,
    const std::vector<sf::Vector2f>& /*otherPlayers*/,
    const sf::Vector2f& /*arenaCenter*/,
    float /*arenaRadius*/
) {
    sf::Vector2f dir{0.f, 0.f};

    // Check for arrow keys first (always available regardless of settings)
    bool arrowKeysUsed = 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    
    if (arrowKeysUsed) {
        // Arrow keys layout
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dir.x += 1.f;
    } else if (Settings::leftyMode) {
        // IJKL layout for lefty mode
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::I)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::K)) dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) dir.x += 1.f;
    } else {
        // WASD layout (default)
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
    }

    return normalize(dir);
}

