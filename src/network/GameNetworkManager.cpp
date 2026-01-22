#include "GameNetworkManager.h"
#include "../core/Settings.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>

GameNetworkManager::GameNetworkManager() {
    initFromEnvironment();
}

void GameNetworkManager::initFromEnvironment() {
    netHost = Settings::onlineHost;
    netPort = static_cast<std::uint16_t>(Settings::onlinePort);

    if (const char* envHost = std::getenv("SUMO_HOST")) {
        netHost = envHost;
    }
    if (const char* envPort = std::getenv("SUMO_PORT")) {
        netPort = static_cast<std::uint16_t>(std::stoi(envPort));
    }
}

void GameNetworkManager::connect(const std::string& host, std::uint16_t port) {
    netHost = host;
    netPort = port;
    
    if (!netClient.connect(netHost, netPort)) {
        std::cerr << "[GameNetworkManager] Failed to connect to " << host << ":" << port << std::endl;
        return;
    }
    
    netConnected = true;
}

void GameNetworkManager::disconnect() {
    netClient.disconnect();
    netConnected = false;
    netJoined = false;
    netPlayers.clear();
    netTargetPositions.clear();
    snapshotBuffer.clear();
}

void GameNetworkManager::service() {
    netClient.service(
        0,
        [&]() { 
            netConnected = true;
            // Send JoinRequest upon connection
            std::vector<std::uint8_t> joinMsg;
            joinMsg.push_back(net::PROTOCOL_VERSION);
            joinMsg.push_back(static_cast<std::uint8_t>(net::MessageType::JoinRequest));
            netClient.send(joinMsg, true);
        },
        [&]() {
            netConnected = false;
            netJoined = false;
            netPlayers.clear();
            rttMs = -1.f;
        },
        [&](const ENetPacket* packet) {
            net::MessageType type;
            if (!net::parseHeader(packet->data, packet->dataLength, type)) return;
            switch (type) {
                case net::MessageType::JoinAccept: {
                    if (packet->dataLength < 2 + sizeof(net::JoinAccept)) return;
                    net::JoinAccept msg{};
                    std::memcpy(&msg, packet->data + 2, sizeof(net::JoinAccept));
                    netPlayerId = msg.playerId;
                    netJoined = true;
                    break;
                }
                case net::MessageType::State: {
                    net::StateSnapshot snap;
                    if (net::deserializeState(packet->data, packet->dataLength, snap)) {
                        applySnapshot(snap);
                    }
                    break;
                }
                case net::MessageType::Pong: {
                    if (packet->dataLength < 2 + sizeof(net::Ping)) return;
                    net::Ping pong{};
                    std::memcpy(&pong, packet->data + 2, sizeof(net::Ping));
                    auto nowMs = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()).count() & 0xFFFFFFFFu);
                    std::uint32_t diff = nowMs - pong.timestampMs;
                    rttMs = static_cast<float>(diff);
                    break;
                }
                default:
                    break;
            }
        }
    );
}

void GameNetworkManager::sendInput(const HumanController& controller, float dt) {
    if (!netJoined || netPlayerId == 0) return;
    auto selfIt = netPlayers.find(netPlayerId);
    if (selfIt == netPlayers.end()) return;

    // Input command construction would go here
    // Simplified for now
    inputSequence++;
}

void GameNetworkManager::sendPing(float dt) {
    pingTimer += dt;
    const float pingInterval = 1.0f;
    if (pingTimer < pingInterval) return;
    pingTimer = 0.f;
    
    net::Ping p{};
    p.timestampMs = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() & 0xFFFFFFFFu);
    lastPingStamp = p.timestampMs;
    netClient.send(net::serializePing(net::MessageType::Ping, p), false);
}

void GameNetworkManager::applySnapshot(const net::StateSnapshot& snap) {
    snapshotBuffer.push_back(snap);
    
    // Update players from snapshot
    for (const auto& sp : snap.players) {
        if (netPlayers.find(sp.playerId) == netPlayers.end()) {
            // New player
            netPlayers.emplace(sp.playerId, Player(sf::Vector2f(sp.x, sp.y), colorForId(sp.playerId)));
        }
        netTargetPositions[sp.playerId] = sf::Vector2f(sp.x, sp.y);
    }
    
    // Remove disconnected players
    for (auto it = netPlayers.begin(); it != netPlayers.end(); ) {
        bool found = false;
        for (const auto& sp : snap.players) {
            if (sp.playerId == it->first) {
                found = true;
                break;
            }
        }
        if (!found) {
            it = netPlayers.erase(it);
        } else {
            ++it;
        }
    }
}

void GameNetworkManager::interpolateSnapshots() {
    // Simplified interpolation - would implement full client-side prediction here
    for (auto& kv : netPlayers) {
        auto targetIt = netTargetPositions.find(kv.first);
        if (targetIt != netTargetPositions.end()) {
            sf::Vector2f current = kv.second.getPosition();
            sf::Vector2f target = targetIt->second;
            
            // Simple lerp towards server position
            float blend = (kv.first == netPlayerId) ? selfCorrectionBlend : 1.0f;
            sf::Vector2f newPos = current + (target - current) * blend;
            kv.second.setPosition(newPos);
        }
    }
}

sf::Color GameNetworkManager::colorForId(std::uint32_t id) const {
    static const sf::Color palette[] = {
        sf::Color::White, sf::Color::Red, sf::Color::Green, sf::Color::Blue,
        sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan,
        sf::Color(255, 128, 0), sf::Color(128, 0, 128), sf::Color(255, 192, 203)
    };
    return palette[id % (sizeof(palette) / sizeof(palette[0]))];
}
