#pragma once
#include "NetClient.h"
#include "NetServer.h"
#include "../game/controllers/HumanController.h"

class GameNetworkManager {
public:
    GameNetworkManager();
    ~GameNetworkManager();

    bool hostGame();
    bool joinGame(const std::string& address, uint16_t port);

    void update(float dt);
    void sendInput(const HumanController& controller, float dt);

    bool isHosting() const { return hosting; }
    bool isConnected() const { return connected; }

private:
    NetServer server;
    NetClient client;
    bool hosting = false;
    bool connected = false;
};
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
