#pragma once

#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <vector>
#include <cstdint>

struct SimPlayer {
    std::uint32_t id{0};
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f velocity{0.f, 0.f};
    sf::Vector2f inputDir{0.f, 0.f};
    bool alive{true};
};

struct SimSnapshotPlayer {
    std::uint32_t id{0};
    sf::Vector2f position{0.f, 0.f};
    sf::Vector2f velocity{0.f, 0.f};
    bool alive{true};
};

class Simulation {
public:
    explicit Simulation(float arenaRadius = 300.f, sf::Vector2f arenaCenter = {600.f, 450.f});

    void setArenaRadius(float r);
    float getArenaRadius() const;

    void addPlayer(std::uint32_t id, sf::Vector2f spawnPos);
    void removePlayer(std::uint32_t id);
    void applyInput(std::uint32_t id, sf::Vector2f dir);

    void tick(float dt);

    std::vector<SimSnapshotPlayer> snapshotPlayers() const;

private:
    sf::Vector2f center;
    float radius;
    std::unordered_map<std::uint32_t, SimPlayer> players;

    void resolveCollisions();
};
