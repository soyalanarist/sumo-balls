#pragma once

#include "../utils/VectorMath.h"
#include <unordered_map>
#include <vector>
#include <cstdint>

struct SimPlayer {
    std::uint32_t id{0};
    Vec2 position{0.f, 0.f};
    Vec2 velocity{0.f, 0.f};
    Vec2 inputDir{0.f, 0.f};
    bool alive{true};
};

struct SimSnapshotPlayer {
    std::uint32_t id{0};
    Vec2 position{0.f, 0.f};
    Vec2 velocity{0.f, 0.f};
    bool alive{true};
};

class Simulation {
public:
    explicit Simulation(float arenaRadius = 300.f, Vec2 arenaCenter = {600.f, 450.f});

    void setArenaRadius(float r);
    float getArenaRadius() const;

    void addPlayer(std::uint32_t id, Vec2 spawnPos);
    void removePlayer(std::uint32_t id);
    void applyInput(std::uint32_t id, Vec2 dir);

    void tick(float dt);

    std::vector<SimSnapshotPlayer> snapshotPlayers() const;

private:
    Vec2 center;
    float radius;
    std::unordered_map<std::uint32_t, SimPlayer> players;

    void resolveCollisions();
};
