#pragma once

#include "utils/VectorMath.h"
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
    explicit Simulation(float arenaRadius = 650.f, Vec2 arenaCenter = {600.f, 450.f});

    void setArenaRadius(float r);
    float getArenaRadius() const;
    float getPlayerRadius() const { return playerRadius; }

    void addPlayer(std::uint32_t id, Vec2 spawnPos);
    void removePlayer(std::uint32_t id);
    void applyInput(std::uint32_t id, Vec2 dir);

    void tick(float dt);

    std::vector<SimSnapshotPlayer> snapshotPlayers() const;
    
    // Arena shrinking
    void updateArenaShrink(float dt);
    float getCurrentArenaRadius() const { return currentArenaRadius; }
    float getArenaAge() const { return arenaAge; }
    
    // Public access to arena parameters
    Vec2 arenaCenter;
    float arenaRadius;  // Initial/maximum radius

private:
    std::unordered_map<std::uint32_t, SimPlayer> players;
    
    // Arena shrinking state
    float arenaAge = 0.0f;           // Time elapsed since arena creation
    float currentArenaRadius;         // Current shrunk radius
    float shrinkStartTime = 3.0f;     // Start shrinking after 3 seconds
    float shrinkRate = 7.0f;          // Shrink 7 units per second (indefinitely)

    // Player parameters
    const float playerRadius = 38.0f;

    void resolveCollisions();
};
