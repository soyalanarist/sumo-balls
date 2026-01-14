#include "AIController.h"
#include "../Player.h"
#include <cmath>

static float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

static sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len == 0.f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}

AIController::AIController(Player* self, std::vector<Player>* players, sf::Vector2f arenaCenter, float arenaRadius):
    self(self),
    players(players),
    arenaCenter(arenaCenter),
    arenaRadius(arenaRadius){}

sf::Vector2f AIController::getMovementDirection(float) {
    sf::Vector2f avoidance{0.f, 0.f};

    // Avoid other players
    for (const Player& other : *players) {
        if (&other == self) continue;

        sf::Vector2f delta = self->getPosition() - other.getPosition();
        float dist = length(delta);

        if (dist > 0.f && dist < 90.f) {
            avoidance += normalize(delta) * (1.f / dist);
        }
    }

    // Avoid arena edge (VERY STRONG)
    sf::Vector2f toCenter = arenaCenter - self->getPosition();

    float distFromEdge = arenaRadius - (std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y) + self->getRadius());
    sf::Vector2f edgeAvoidance = normalize(toCenter) * (distFromEdge < 100.f ? (1.f / distFromEdge) : 0.f);

    sf::Vector2f finalDir = avoidance * 0.2f + edgeAvoidance * 3.0f;
    return normalize(finalDir);
}
