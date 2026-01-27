#include "network/NetServer.h"
#include "network/NetProtocol.h"
#include "game/simulation/Simulation.h"

#include "utils/VectorMath.h"
#include <iostream>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <cmath>

struct ClientInfo {
    std::uint32_t playerId{0};
};

int main(int argc, char** argv) {
    std::uint16_t port = 7777;
    if (argc >= 2) {
        port = static_cast<std::uint16_t>(std::stoi(argv[1]));
    }

    net::NetServer server;
    if (!server.start(port, 8)) {
        std::cerr << "Failed to start server on port " << port << "\n";
        return 1;
    }
    std::cout << "Authoritative server listening on port " << port << "\n";

    Simulation sim(300.f, {600.f, 450.f});
    std::unordered_map<ENetPeer*, ClientInfo> peers;
    std::uint32_t nextPlayerId = 1;

    auto startTime = std::chrono::steady_clock::now();
    auto last = startTime;
    float accumulator = 0.f;
    const float fixedDt = 1.f / 60.f;
    float snapshotTimer = 0.f;
    std::uint32_t tick = 0;

    auto onConnect = [&](ENetPeer* peer) {
        std::uint32_t id = nextPlayerId++;
        peers[peer] = ClientInfo{id};

        // Spawn players in a ring
        const float angle = static_cast<float>(id % 6) / 6.f * 6.2831853f;
        Vec2 spawn(600.f + 200.f * std::cos(angle), 450.f + 200.f * std::sin(angle));
        sim.addPlayer(id, spawn);

        net::JoinAccept msg{ id };
        server.sendTo(peer, net::serializeJoinAccept(msg), true);
        std::cout << "Client connected, assigned playerId=" << id << "\n";
    };

    auto onDisconnect = [&](ENetPeer* peer) {
        auto it = peers.find(peer);
        if (it != peers.end()) {
            sim.removePlayer(it->second.playerId);
            peers.erase(it);
            std::cout << "Client disconnected\n";
        }
    };

    auto onPacket = [&](ENetPeer* peer, const ENetPacket* packet) {
        if (!packet || packet->dataLength < 2) return;
        net::MessageType type;
        if (!net::parseHeader(packet->data, packet->dataLength, type)) return;

        switch (type) {
            case net::MessageType::Input: {
                if (packet->dataLength < 2 + sizeof(net::InputCommand)) return;
                net::InputCommand cmd{};
                std::memcpy(&cmd, packet->data + 2, sizeof(net::InputCommand));
                sim.applyInput(cmd.playerId, {cmd.dirX, cmd.dirY});
                break;
            }
            case net::MessageType::Ping: {
                if (packet->dataLength < 2 + sizeof(net::Ping)) return;
                net::Ping ping{};
                std::memcpy(&ping, packet->data + 2, sizeof(net::Ping));
                auto data = net::serializePing(net::MessageType::Pong, ping);
                server.sendTo(peer, data, false);
                break;
            }
            default:
                break;
        }
    };

    while (true) {
        server.service(0, onConnect, onDisconnect, onPacket);

        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last).count();
        last = now;
        accumulator += dt;
        while (accumulator >= fixedDt) {
            sim.tick(fixedDt);
            accumulator -= fixedDt;
            ++tick;
            snapshotTimer += fixedDt;
        }

        if (snapshotTimer >= 0.03f) {  // 30ms between snapshots (33 per second)
            snapshotTimer = 0.f;
            net::StateSnapshot snap;
            snap.tick = tick;
            auto now = std::chrono::steady_clock::now();
            snap.serverTimeMs = static_cast<std::uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count());
            snap.arenaRadius = sim.getArenaRadius();
            for (const auto& p : sim.snapshotPlayers()) {
                net::PlayerState ps{};
                ps.playerId = p.id;
                ps.x = p.position.x;
                ps.y = p.position.y;
                ps.vx = p.velocity.x;
                ps.vy = p.velocity.y;
                ps.alive = p.alive ? 1 : 0;
                snap.players.push_back(ps);
            }
            std::cout << "Server: Broadcasting snapshot with " << snap.players.size() << " players, serverTime=" << snap.serverTimeMs << "\n";
            server.broadcast(net::serializeState(snap), false);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    return 0;
}
