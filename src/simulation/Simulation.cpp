#include "Simulation.h"

#include <cmath>

namespace {
float magnitude(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = magnitude(v);
    if (len <= 0.00001f) return {0.f, 0.f};
    return {v.x / len, v.y / len};
}
}

Simulation::Simulation(float arenaRadius, sf::Vector2f arenaCenter)
    : center(arenaCenter), radius(arenaRadius){}

void Simulation::setArenaRadius(float r) { radius = r; }
float Simulation::getArenaRadius() const { return radius; }

void Simulation::addPlayer(std::uint32_t id, sf::Vector2f spawnPos){
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

void Simulation::applyInput(std::uint32_t id, sf::Vector2f dir) {
    auto it = players.find(id);
    if (it == players.end()) return;
    it->second.inputDir = normalize(dir);
}

void Simulation::tick(float dt) {
    // Basic movement params (mirrors Player.cpp roughly)
    constexpr float speed = 100.f;
    constexpr float acceleration = 25.f;
    constexpr float friction = 0.002f;
    constexpr float maxSpeed = 500.f;
    constexpr float playerRadius = 18.f;

    for (auto& kv : players) {
        auto& p = kv.second;
        if (!p.alive) continue;

        p.velocity += p.inputDir * speed * acceleration * dt;
        p.velocity *= (1.f - friction);

        float spd = magnitude(p.velocity);
        if (spd > maxSpeed) {
            p.velocity = (p.velocity / spd) * maxSpeed;
        }

        p.position += p.velocity * dt;

        // Death if too far outside arena (50% of radius buffer)
        sf::Vector2f toCenter = p.position - center;
        float dist = magnitude(toCenter);
        float deathDist = radius + playerRadius * 0.5f;
        if (dist > deathDist) {
            p.alive = false;
            p.velocity = {0.f, 0.f};
        }
    }

    resolveCollisions();
}

void Simulation::resolveCollisions() {
    constexpr float playerRadius = 18.f;
    constexpr float restitution = 2.05f;

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
            float push = overlap * 0.5f + 3.0f;
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

            sf::Vector2f newVa(newVaN * nx + vaT * (-ny), newVaN * ny + vaT * nx);
            sf::Vector2f newVb(newVbN * nx + vbT * (-ny), newVbN * ny + vbT * nx);

            constexpr float impulseBoost = 1.15f;
            pa->velocity += (newVa - pa->velocity) * impulseBoost;
            pb->velocity += (newVb - pb->velocity) * impulseBoost;
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
