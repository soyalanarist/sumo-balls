#pragma once
#include "NetClient.h"
#include "NetProtocol.h"
#include "../game/entities/Player.h"
#include "../game/controllers/HumanController.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <deque>
#include <cstdint>

/// Manages all network communication for a game session
/// Handles connection, input sending, snapshot receiving, and interpolation
class GameNetworkManager {
public:
    GameNetworkManager();
    ~GameNetworkManager() = default;

    // Configuration
    void initFromEnvironment();
    void connect(const std::string& host, std::uint16_t port);
    void disconnect();
    
    // State
    bool isConnected() const { return netConnected; }
    bool isJoined() const { return netJoined; }
    std::uint32_t getPlayerId() const { return netPlayerId; }
    float getRtt() const { return rttMs; }
    
    // Network operations
    void service();  // Poll network events
    void sendInput(const HumanController& controller, float dt);
    void sendPing(float dt);
    
    // Players
    const std::unordered_map<std::uint32_t, Player>& getPlayers() const { return netPlayers; }
    const std::unordered_map<std::uint32_t, sf::Vector2f>& getTargetPositions() const { return netTargetPositions; }
    
    // Interpolation
    void interpolateSnapshots();
    float getSelfCorrectionBlend() const { return selfCorrectionBlend; }
    void setSelfCorrectionBlend(float blend) { selfCorrectionBlend = blend; }

private:
    // Network state
    std::string netHost = "127.0.0.1";
    std::uint16_t netPort = 7777;
    net::NetClient netClient;
    bool netConnected = false;
    bool netJoined = false;
    std::uint32_t netPlayerId = 0;
    std::uint32_t inputSequence = 1;
    
    // Players
    std::unordered_map<std::uint32_t, Player> netPlayers;
    std::unordered_map<std::uint32_t, sf::Vector2f> netTargetPositions;
    
    // Snapshot buffering and interpolation
    std::deque<net::StateSnapshot> snapshotBuffer;
    float interpDelayMs = 0.f;  // No delay for smoothest rendering
    float selfCorrectionBlend = 0.35f; // How strongly to correct toward server for self
    
    // RTT measurement
    float rttMs = -1.f;
    float pingTimer = 0.f;
    std::uint32_t lastPingStamp = 0;
    
    // Helper methods
    void applySnapshot(const net::StateSnapshot& snap);
    sf::Color colorForId(std::uint32_t id) const;
};
