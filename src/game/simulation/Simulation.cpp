#include "Simulation.h"
#include "PhysicsValidator.h"
#include "utils/VectorMath.h"

#include <cmath>
#include <iostream>

namespace {
float magnitude(const Vec2& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vec2 normalize(const Vec2& v) {
    float len = magnitude(v);
    if (len <= 0.00001f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}
}

Simulation::Simulation(float arenaRadius, Vec2 arenaCenter)
    : arenaCenter(arenaCenter), arenaRadius(arenaRadius), currentArenaRadius(arenaRadius) {}

void Simulation::setArenaRadius(float r) { arenaRadius = r; }
float Simulation::getArenaRadius() const { return arenaRadius; }

void Simulation::addPlayer(std::uint32_t id, Vec2 spawnPos){
    SimPlayer p;
    p.id = id;
    p.position = spawnPos;
    p.velocity = {0.f, 0.f};
    p.inputDir = {0.f, 0.f};
    p.alive = true;
    players[id] = p;
}

void Simulation::removePlayer(std::uint32_t id) {
    players.erase(id);
}

void Simulation::applyInput(std::uint32_t id, Vec2 dir) {
    auto it = players.find(id);
    if (it == players.end()) return;
    it->second.inputDir = normalize(dir);
}

void Simulation::updateArenaShrink(float dt) {
    arenaAge += dt;
    
    if (arenaAge < shrinkStartTime) {
        // No shrinking yet
        currentArenaRadius = arenaRadius;
    } else {
        // Shrink continuously at constant rate
        float timeShrinking = arenaAge - shrinkStartTime;
        currentArenaRadius = arenaRadius - (shrinkRate * timeShrinking);
        
        // Don't let radius go below 10 (avoid division by zero, but effectively game-ending)
        if (currentArenaRadius < 10.0f) {
            currentArenaRadius = 10.0f;
        }
    }
}

void Simulation::tick(float dt) {
    // Basic movement params (mirrors Player.cpp roughly)
    constexpr float speed = 180.f;          // higher base thrust
    constexpr float acceleration = 36.f;    // quicker acceleration
    constexpr float friction = 0.0015f;     // less damping for sustained motion
    constexpr float maxSpeed = 620.f;       // allow faster top speed

    for (auto& kv : players) {
        auto& p = kv.second;
        if (!p.alive) continue;

        p.velocity += p.inputDir * speed * acceleration * dt;
        p.velocity *= (1.f - friction);

        float spd = magnitude(p.velocity);
        if (spd > maxSpeed) {
            p.velocity = (p.velocity / spd) * maxSpeed;
        }

        p.position = p.position + p.velocity * dt;
        
        // Validate physics state
        PhysicsValidator::validateAndClampPosition(p.position);
        PhysicsValidator::validateAndClampVelocity(p.velocity);

        // Death if too far outside arena (using current shrunk radius)
        Vec2 toCenter = p.position - arenaCenter;
        float dist = magnitude(toCenter);
        float deathDist = currentArenaRadius + playerRadius * 0.35f;
        if (dist > deathDist) {
            p.alive = false;
            p.velocity = {0.f, 0.f};
        }
    }

    resolveCollisions();
}

void Simulation::resolveCollisions() {
    const float restitution = 2.15f;    // snappier bounce

    std::vector<std::uint32_t> ids;
    ids.reserve(players.size());
    for (const auto& kv : players) ids.push_back(kv.first);

    for (size_t a = 0; a < ids.size(); ++a) {
        for (size_t b = a + 1; b < ids.size(); ++b) {
            auto* pa = &players[ids[a]];
            auto* pb = &players[ids[b]];
            if (!pa->alive || !pb->alive) continue;

            float dx = pb->position.x - pa->position.x;
            float dy = pb->position.y - pa->position.y;
            float distSq = dx * dx + dy * dy;
            float minDist = playerRadius * 2.f;
            float minDistSq = minDist * minDist;
            if (distSq >= minDistSq || distSq <= 0.000001f) continue;

            float dist = std::sqrt(distSq);
            float overlap = minDist - dist;
            float push = overlap * 0.6f + 4.5f; // stronger separation
            float nx = dx / dist;
            float ny = dy / dist;

            pa->position.x -= nx * push;
            pa->position.y -= ny * push;
            pb->position.x += nx * push;
            pb->position.y += ny * push;

            float vaN = pa->velocity.x * nx + pa->velocity.y * ny;
            float vbN = pb->velocity.x * nx + pb->velocity.y * ny;
            float vaT = pa->velocity.x * (-ny) + pa->velocity.y * nx;
            float vbT = pb->velocity.x * (-ny) + pb->velocity.y * nx;
            if (vaN - vbN <= 0.f) continue;

            float newVaN = ((vaN + vbN) + restitution * (vbN - vaN)) * 0.5f;
            float newVbN = ((vaN + vbN) + restitution * (vaN - vbN)) * 0.5f;

            Vec2 newVa(newVaN * nx + vaT * (-ny), newVaN * ny + vaT * nx);
            Vec2 newVb(newVbN * nx + vbT * (-ny), newVbN * ny + vbT * nx);

            constexpr float impulseBoost = 1.25f; // slightly stronger collision impulse
            Vec2 impulseA = (newVa - pa->velocity) * impulseBoost;
            Vec2 impulseB = (newVb - pb->velocity) * impulseBoost;
            
            // Validate impulses before applying
            if (!PhysicsValidator::isVelocityValid(impulseA) || 
                !PhysicsValidator::isVelocityValid(impulseB)) {
                std::cerr << "[Simulation] Invalid collision impulse detected, skipping" << std::endl;
                continue;
            }
            
            pa->velocity += impulseA;
            pb->velocity += impulseB;
        }
    }
}

std::vector<SimSnapshotPlayer> Simulation::snapshotPlayers() const {
    std::vector<SimSnapshotPlayer> out;
    out.reserve(players.size());
    for (const auto& kv : players) {
        const auto& p = kv.second;
        SimSnapshotPlayer s;
        s.id = p.id;
        s.position = p.position;
        s.velocity = p.velocity;
        s.alive = p.alive;
        out.push_back(s);
    }
    return out;
}
