#pragma once
#include "../core/Screen.h"
#include "../game/entities/PlayerEntity.h"
#include "../game/entities/Arena.h"
#include "menus/MenuAction.h"
#include "../network/NetClient.h"
#include "../network/NetProtocol.h"
#include "../game/entities/Player.h"
#include "../game/controllers/HumanController.h"
#include <SFML/Graphics.hpp>
#include <deque>
#include <unordered_map>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;        // Total lifetime in seconds
    float timeRemaining;   // Time left before particle dies
    float radius;
};

class GameScreen : public Screen {
public:
    GameScreen();

    void update(sf::Time deltaTime, sf::RenderWindow& window) override;
    void render(sf::RenderWindow& window) override;
    bool isOverlay() const override { return false; }

    void resolvePlayerCollisions();
    void createExplosion(sf::Vector2f position, sf::Vector2f velocity);
    void updateParticles(float dt);
    
    MenuAction getMenuAction() const override;
    void resetMenuAction() override;

private:
    // Offline/local state
    Arena arena;
    std::vector<PlayerEntity> players;
    std::vector<sf::Vector2f> positions;  // Cache for player positions
    std::vector<Particle> particles;      // Explosion particles
    int frameCount = 0;  // Instance member instead of static
    float gameTime = 0.f;                 // Total elapsed game time
    float countdownTime = 3.0f;           // Countdown before game starts
    bool countdownActive = true;          // Whether countdown is running
    bool gameOver = false;
    float gameOverTime = 0.f;             // Time since game ended
    float initialArenaRadius = 300.f;     // Starting arena radius for speed scaling
    MenuAction menuAction = MenuAction::NONE;  // Track requested menu action
    float getSpeedMultiplier() const;     // Calculate speed multiplier based on shrinkage
    sf::Font font;                        // Cached font for countdown text

    // Online session state (authoritative server)
    bool onlineMode = false;
    std::string netHost = "127.0.0.1";
    std::uint16_t netPort = 7777;
    net::NetClient netClient;
    bool netConnected = false;
    bool netJoined = false;
    std::uint32_t netPlayerId = 0;
    std::uint32_t inputSequence = 1;
    std::unordered_map<std::uint32_t, Player> netPlayers;
    std::unordered_map<std::uint32_t, sf::Vector2f> netTargetPositions;
    std::deque<net::StateSnapshot> snapshotBuffer;
    float interpDelayMs = 0.f;  // No delay for smoothest rendering
    float rttMs = -1.f;
    float pingTimer = 0.f;
    std::uint32_t lastPingStamp = 0;
    HumanController netController;
    float selfCorrectionBlend = 0.35f; // how strongly to correct toward server for self

    void initOnlineConfigFromEnv();
    void handleNetService();
    void sendNetInput(float dt);
    void sendNetPing(float dt);
    void applySnapshot(const net::StateSnapshot& snap);
    void interpolateSnapshots();
    sf::Color colorForId(std::uint32_t id) const;
};
