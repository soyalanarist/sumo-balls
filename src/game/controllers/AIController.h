#pragma once

#include "Controller.h"
#include <SFML/System/Vector2.hpp>
#include <vector>

// forward declaration of Player class for compilation efficiency
class Player;

class AIController : public Controller {
public:
    AIController(Player* self, std::vector<Player>* players, sf::Vector2f arenaCenter, float arenaRadius);

    sf::Vector2f getMovementDirection(float dt) override;

private:
    Player* self;
    std::vector<Player> *players;

    sf::Vector2f arenaCenter;
    float arenaRadius;
};
