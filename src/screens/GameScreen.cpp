#include "GameScreen.h"
#include "menus/GameOverMenu.h"
#include "../game/GamePhase.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../core/Settings.h"
#include "../utils/GameConstants.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <thread>
#include <random>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>

GameScreen::GameScreen(bool forceOffline):
    arena({GameConstants::ARENA_CENTER_X, GameConstants::ARENA_CENTER_Y}, GameConstants::ARENA_RADIUS),
    initialArenaRadius(GameConstants::ARENA_RADIUS){
    
    std::cout << "[GameScreen] Constructor starting (forceOffline=" << forceOffline << ")" << std::endl;
    
    // Load font once at initialization
    if(!font.loadFromFile("assets/arial.ttf")) {
        throw std::runtime_error("Failed to load font: assets/arial.ttf");
    }
    
    std::cout << "[GameScreen] Font loaded" << std::endl;

    // Short-circuit for offline single-player
    if(forceOffline) {
        onlineMode = false;
        gamePhase = GamePhase::Playing;
        countdownTime = 0.f;
        initializeLocalGame();
        std::cout << "[GameScreen] Forced offline start (single-player)" << std::endl;
        return;
    }

    initOnlineConfigFromEnv();
    std::cout << "[GameScreen] Online config initialized, onlineMode=" << onlineMode << std::endl;

    // Attempt to connect with a shorter timeout
    const int maxAttempts = 50;  // ~1 second at 20ms interval
    int attempts = 0;
    while (attempts < maxAttempts) {
        if (!netConnected) {
            netClient.connect(netHost, netPort);
        }
        handleNetService();
        if (netConnected) {
            gamePhase = GamePhase::Playing;  // Online mode starts in playing state
            onlineMode = true;
            std::cout << "[GameScreen] Connected to server in online mode" << std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        ++attempts;
    }

    // Fallback to single-player mode if connection fails
    onlineMode = false;
    gamePhase = GamePhase::Countdown;
    countdownTime = 3.0f;
    std::cout << "[GameScreen] Failed to connect to server, starting in single-player mode" << std::endl;
    
    // Initialize local single-player game with AI opponents
    initializeLocalGame();
    std::cout << "[GameScreen] Constructor completed successfully" << std::endl;
}

void GameScreen::initializeLocalGame() {
    // Create human player (player 0) with white color
    sf::Vector2f humanStartPos = arena.getCenter() + sf::Vector2f(-100.f, 0.f);
    auto humanController = std::make_unique<HumanController>();
    players.emplace_back(humanStartPos, std::move(humanController), sf::Color::White);
    
    // Create 5 AI opponents with different colors and difficulties
    sf::Color aiColors[] = {
        sf::Color::Red,
        sf::Color::Green,
        sf::Color::Blue,
        sf::Color::Yellow,
        sf::Color::Magenta
    };
    
    float difficulties[] = {0.3f, 0.4f, 0.5f, 0.6f, 0.7f};
    
    for(int i = 0; i < 5; ++i) {
        // Distribute AI players around the arena
        float angle = (2.f * 3.14159f / 5.f) * i;
        float distance = 150.f;
        sf::Vector2f aiPos = arena.getCenter() + sf::Vector2f(
            distance * std::cos(angle),
            distance * std::sin(angle)
        );
        
        auto aiController = std::make_unique<AIController>(difficulties[i]);
        players.emplace_back(aiPos, std::move(aiController), aiColors[i]);
    }
    
    std::cout << "[GameScreen] Initialized local game with 1 player + 5 AI opponents" << std::endl;
}

void GameScreen::initOnlineConfigFromEnv() {
    onlineMode = true;  // Online-only mode enforced
    netHost = Settings::onlineHost;
    netPort = static_cast<std::uint16_t>(Settings::onlinePort);

    // Allow host/port overrides via env for flexibility
    if (const char* envHost = std::getenv("SUMO_HOST")) {
        netHost = envHost;
    }
    if (const char* envPort = std::getenv("SUMO_PORT")) {
        netPort = static_cast<std::uint16_t>(std::stoi(envPort));
    }
}

sf::Color GameScreen::colorForId(std::uint32_t id) const {
    // Deterministic palette based on id
    static const sf::Color palette[] = {
        sf::Color::White, sf::Color::Red, sf::Color::Green, sf::Color::Blue,
        sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan,
        sf::Color(255, 128, 0), sf::Color(128, 0, 128), sf::Color(255, 192, 203)
    };
    return palette[id % (sizeof(palette) / sizeof(palette[0]))];
}

void GameScreen::handleNetService() {
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

void GameScreen::sendNetInput(float dt) {
    if (!netJoined || netPlayerId == 0) return;
    auto selfIt = netPlayers.find(netPlayerId);
    if (selfIt == netPlayers.end()) return;

    std::vector<sf::Vector2f> others;
    others.reserve(netPlayers.size() > 0 ? netPlayers.size() - 1 : 0);
    for (const auto& kv : netPlayers) {
        if (kv.first == netPlayerId) continue;
        others.push_back(kv.second.getPosition());
    }

    sf::Vector2f dir = netController.getMovementDirection(
        dt,
        selfIt->second.getPosition(),
        others,
        arena.center,
        arena.radius
    );

    net::InputCommand cmd{};
    cmd.playerId = netPlayerId;
    cmd.dirX = dir.x;
    cmd.dirY = dir.y;
    cmd.sequence = inputSequence++;
    cmd.timestampMs = static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count() & 0xFFFFFFFFu);
    netClient.send(net::serializeInput(cmd), false);

    // Client-side prediction: advance local player with same input
    selfIt->second.setMovementDirection(dir);
    selfIt->second.update(dt, getSpeedMultiplier());
}

void GameScreen::sendNetPing(float dt) {
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

void GameScreen::applySnapshot(const net::StateSnapshot& snap) {
    arena.setRadius(snap.arenaRadius);
    snapshotBuffer.push_back(snap);
    while (snapshotBuffer.size() > 30) snapshotBuffer.pop_front();

    for (const auto& ps : snap.players) {
        auto it = netPlayers.find(ps.playerId);
        if (it == netPlayers.end()) {
            Player p({ps.x, ps.y}, colorForId(ps.playerId));
            p.setAlive(ps.alive != 0);
            netPlayers.emplace(ps.playerId, std::move(p));
        } else {
            sf::Vector2f authPos{ps.x, ps.y};
            if (ps.playerId == netPlayerId) {
                // Reconcile local prediction toward authoritative position
                sf::Vector2f cur = it->second.getPosition();
                sf::Vector2f corrected{
                    cur.x + (authPos.x - cur.x) * selfCorrectionBlend,
                    cur.y + (authPos.y - cur.y) * selfCorrectionBlend
                };
                it->second.setPosition(corrected);
            } else {
                it->second.setPosition(authPos);
            }
            it->second.setAlive(ps.alive != 0);
        }
        netTargetPositions[ps.playerId] = {ps.x, ps.y};
    }
}

void GameScreen::interpolateSnapshots() {
    if (snapshotBuffer.empty()) return;
    const auto& latest = snapshotBuffer.back();
    std::uint32_t renderTime = (latest.serverTimeMs > interpDelayMs) ? static_cast<std::uint32_t>(latest.serverTimeMs - interpDelayMs) : latest.serverTimeMs;

    const net::StateSnapshot* prev = nullptr;
    const net::StateSnapshot* next = nullptr;
    for (const auto& s : snapshotBuffer) {
        if (s.serverTimeMs <= renderTime) prev = &s;
        if (s.serverTimeMs >= renderTime) { next = &s; break; }
    }
    if (!prev) prev = &snapshotBuffer.front();
    if (!next) next = &snapshotBuffer.back();

    float alpha = 0.f;
    if (next != prev && next->serverTimeMs != prev->serverTimeMs) {
        alpha = static_cast<float>(renderTime - prev->serverTimeMs) / static_cast<float>(next->serverTimeMs - prev->serverTimeMs);
        if (alpha < 0.f) alpha = 0.f;
        if (alpha > 1.f) alpha = 1.f;
    }

    arena.setRadius(next->arenaRadius);

    auto applyLerp = [&](const net::StateSnapshot* A, const net::StateSnapshot* B, float t) {
        // Index players by id for quick lookup
        std::unordered_map<std::uint32_t, const net::PlayerState*> mapA;
        mapA.reserve(A->players.size());
        for (const auto& p : A->players) mapA[p.playerId] = &p;
        for (const auto& pb : B->players) {
            // Skip interpolation for self player - using client-side prediction instead
            if (pb.playerId == netPlayerId) continue;
            
            const auto* pa = mapA.count(pb.playerId) ? mapA[pb.playerId] : nullptr;
            sf::Vector2f posB{pb.x, pb.y};
            sf::Vector2f posA = pa ? sf::Vector2f{pa->x, pa->y} : posB;
            
            // Smooth interpolation with slight extrapolation for more fluid movement
            float smoothT = t * t * (3.f - 2.f * t);  // Smoothstep function
            sf::Vector2f lerped{
                posA.x + (posB.x - posA.x) * smoothT,
                posA.y + (posB.y - posA.y) * smoothT
            };
            
            auto it = netPlayers.find(pb.playerId);
            if (it != netPlayers.end()) {
                it->second.setPosition(lerped);
                it->second.setAlive(pb.alive != 0);
            }
        }
    };

    applyLerp(prev, next, alpha);
}

void GameScreen::transitionToPhase(GamePhase newPhase) {
    if (isValidTransition(gamePhase, newPhase)) {
        gamePhase = newPhase;
    }
}

void GameScreen::updateGamePhase(sf::Time dt) {
    // Single-player game loop
    gameTime += dt.asSeconds();
    
    switch(gamePhase) {
        case GamePhase::Countdown: {
            countdownTime -= dt.asSeconds();
            if(countdownTime <= 0.f) {
                transitionToPhase(GamePhase::Playing);
            }
            break;
        }
        case GamePhase::Playing: {
            // Update arena shrinking a bit faster for visibility
            float shrinkRate = 30.f;  // pixels per second
            float newRadius = arena.getRadius() - shrinkRate * dt.asSeconds();
            arena.setRadius(std::max(50.f, newRadius));
            
            // Collect player positions
            std::vector<sf::Vector2f> positions;
            for(const auto& player : players) {
                if(player.isAlive()) {
                    positions.push_back(player.getPosition());
                }
            }
            
            // Update all players with AI/human control
            for(size_t i = 0; i < players.size(); ++i) {
                if(!players[i].isAlive()) continue;
                std::vector<sf::Vector2f> otherPositions;
                for(size_t j = 0; j < positions.size(); ++j) {
                    if(i != j) {
                        otherPositions.push_back(positions[j]);
                    }
                }
                
                float speedMult = getSpeedMultiplier();
                players[i].update(dt.asSeconds(), otherPositions, arena.getCenter(), arena.getRadius(), speedMult);

                // Eliminate players that leave the arena
                if(!arena.contains(players[i].getPosition(), -players[i].getRadius() * 0.2f)) {
                    createExplosion(players[i].getPosition(), players[i].getVelocity(), players[i].getColor());
                    players[i].setAlive(false);
                }
            }
            
            // Handle collisions
            resolvePlayerCollisions();
            
            // Check if only one player remains
            int aliveCount = 0;
            for(size_t i = 0; i < players.size(); ++i) {
                if(players[i].isAlive()) {
                    aliveCount++;
                }
            }
            
            // Game ends when arena is too small or only one player left
            if(arena.getRadius() <= 50.f || aliveCount <= 1) {
                transitionToPhase(GamePhase::GameOver);
                gameOverTime = 0.f;
            }

            // Debug: log offline state periodically
            static float debugTimer = 0.f;
            debugTimer += dt.asSeconds();
            if(debugTimer >= 1.f) {
                std::cout << "[SP] radius=" << arena.getRadius() << " alive=" << aliveCount << std::endl;
                debugTimer = 0.f;
            }
            break;
        }
        case GamePhase::GameOver: {
            gameOverTime += dt.asSeconds();
            break;
        }
        case GamePhase::Paused: {
            // Paused state - do nothing
            break;
        }
    }

    // Update particle effects every frame (offline)
    updateParticles(dt.asSeconds());
}

void GameScreen::update(sf::Time dt, [[maybe_unused]] sf::RenderWindow& window) {
    try{
        frameCount++;
        
        // Online client mode: process networking FIRST, before any gameplay
        if(onlineMode && !netConnected) {
            throw std::runtime_error("Lost connection in online mode");
        }

        if(onlineMode) {
            handleNetService();
            sendNetInput(dt.asSeconds());
            sendNetPing(dt.asSeconds());
            interpolateSnapshots();
        } else {
            // Single-player mode: update local game state
            // For now, just handle arena shrinking and game loop
            updateGamePhase(dt);
        }
        return;
    } catch(const std::exception& e) {
        std::cerr << "GameScreen update error: " << e.what() << std::endl;
        throw;
    } catch(...) {
        std::cerr << "Unknown GameScreen update error" << std::endl;
        throw;
    }
}

float GameScreen::getSpeedMultiplier() const {
    // Speed multiplier increases as arena shrinks
    // At full size: 1.0x, at half size: 1.5x, at 1/4 size: 2.0x
    float currentRadius = arena.radius;
    if(initialArenaRadius <= 0.f) return 1.0f;
    float shrinkRatio = currentRadius / initialArenaRadius;
    // Linear scaling: 1.0 + (1 - shrinkRatio)
    return 1.0f + (1.0f - shrinkRatio);
}

void GameScreen::render(sf::RenderWindow& window) {
    try {
        arena.render(window);
        
        if(onlineMode) {
            for(auto& kv : netPlayers) {
                if(kv.second.isAlive()) {
                    kv.second.render(window);
                }
            }
        } else {
            // Single-player mode: render local players
            for(auto& player : players) {
                if(player.isAlive()) {
                    player.render(window);
                }
            }
            // Render particle effects for eliminations
            for(const auto& p : particles) {
                sf::CircleShape c;
                c.setRadius(p.radius);
                c.setOrigin(p.radius, p.radius);
                c.setPosition(p.position);
                // Use color-dependent particle color (use player's color with some transparency)
                sf::Color particleColor = p.color;
                particleColor.a = static_cast<sf::Uint8>(180 * (p.timeRemaining / p.lifetime));  // Fade out
                c.setFillColor(particleColor);
                window.draw(c);
            }
        }
        
        // HUD: show game phase and status
        sf::Text gameInfo;
        gameInfo.setFont(font);
        gameInfo.setCharacterSize(18);
        gameInfo.setFillColor(sf::Color::White);
        
        std::string statusStr;
        if(onlineMode) {
            std::string rttStr = (rttMs >= 0.f) ? std::to_string(static_cast<int>(rttMs)) + " ms" : "--";
            statusStr = "Online  RTT: " + rttStr;
        } else {
            switch(gamePhase) {
                case GamePhase::Countdown:
                    statusStr = "Starting in " + std::to_string(static_cast<int>(countdownTime + 1)) + "...";
                    break;
                case GamePhase::Playing:
                    statusStr = "Single-Player";
                    break;
                case GamePhase::GameOver:
                    statusStr = "Game Over";
                    break;
                default:
                    statusStr = "Paused";
            }
        }
        
        gameInfo.setString(statusStr);
        gameInfo.setPosition(10.f, 10.f);
        window.draw(gameInfo);
        return;
    } catch(const std::exception& e) {
        std::cerr << "GameScreen render error: " << e.what() << std::endl;
        throw;
    } catch(...) {
        std::cerr << "Unknown GameScreen render error" << std::endl;
        throw;
    }
}

MenuAction GameScreen::getMenuAction() const {
    // Return pause action if requested
    if(menuAction == MenuAction::PAUSE) {
        return MenuAction::PAUSE;
    }
    
    // Return main menu action after game is over and particles finished
    if(gamePhase == GamePhase::GameOver && gameOverTime >= 0.7f) {
        return MenuAction::MAIN_MENU;
    }
    
    return MenuAction::NONE;
}

void GameScreen::resetMenuAction() {
    menuAction = MenuAction::NONE;
}

void GameScreen::createExplosion(sf::Vector2f position, sf::Vector2f velocity, sf::Color color) {
    const int PARTICLE_COUNT = 18;
    const float PARTICLE_LIFETIME = 0.6f;
    const float PARTICLE_RADIUS = 4.f;
    const float VELOCITY_MULTIPLIER = 1.5f;
    
    // Get magnitude of player velocity to use as base for particle spread
    float velocityMagnitude = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if(velocityMagnitude < 10.f) {
        velocityMagnitude = 50.f;  // Minimum spread if player was barely moving
    }
    
    // Create particles in radial pattern
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        // Angle for this particle (0 to 360 degrees, spread evenly)
        float angle = (360.f / PARTICLE_COUNT) * i;
        float radians = angle * 3.14159265f / 180.f;
        
        // Base direction from angle
        float dirX = std::cos(radians);
        float dirY = std::sin(radians);
        
        // Add player's velocity direction component for more dynamic effect
        float velocityX = velocity.x + (dirX * velocityMagnitude * VELOCITY_MULTIPLIER);
        float velocityY = velocity.y + (dirY * velocityMagnitude * VELOCITY_MULTIPLIER);
        
        particles.emplace_back(Particle{
            position,
            sf::Vector2f(velocityX, velocityY),
            PARTICLE_LIFETIME,
            PARTICLE_LIFETIME,
            PARTICLE_RADIUS,
            color  // Use the player's color for particles
        });
    }
}

void GameScreen::updateParticles(float dt) {
    for(auto it = particles.begin(); it != particles.end(); ) {
        it->timeRemaining -= dt;
        
        if(it->timeRemaining <= 0.f) {
            it = particles.erase(it);
        } else {
            // Update position
            it->position.x += it->velocity.x * dt;
            it->position.y += it->velocity.y * dt;
            
            // Apply slight gravity/drag
            it->velocity.y += 50.f * dt;  // Gravity
            it->velocity.x *= 0.98f;      // Drag
            it->velocity.y *= 0.98f;
            
            ++it;
        }
    }
}

void GameScreen::resolvePlayerCollisions() {
    const float RESTITUTION = 2.05f;  // Higher elasticity for stronger rebounds
    
    for(size_t i = 0; i < players.size(); i++) {
        for(size_t j = i + 1; j < players.size(); j++) {
            if(!players[i].isAlive() || !players[j].isAlive()) continue;
            
            sf::Vector2f pos1 = players[i].getPosition();
            sf::Vector2f pos2 = players[j].getPosition();
            
            float radius1 = players[i].getRadius();
            float radius2 = players[j].getRadius();
            float minDistance = radius1 + radius2;
            
            // Use squared distance for faster collision check (avoid sqrt)
            float dx = pos2.x - pos1.x;
            float dy = pos2.y - pos1.y;
            float distSq = dx * dx + dy * dy;
            float minDistSq = minDistance * minDistance;
            
            if(distSq < minDistSq && distSq > 0.000001f) {
                // Collision detected - now calculate actual distance for resolution
                float distance = std::sqrt(distSq);
                float overlap = minDistance - distance;
                float pushDistance = overlap * 0.5f + 3.0f;  // Extra separation force
                
                // Normalized collision normal
                float nx = dx / distance;
                float ny = dy / distance;
                
                // Separate overlapping balls with more force
                players[i].move(sf::Vector2f(-nx * pushDistance, -ny * pushDistance));
                players[j].move(sf::Vector2f(nx * pushDistance, ny * pushDistance));
                
                // Get masses and velocities
                float mass1 = players[i].getMass();
                float mass2 = players[j].getMass();
                sf::Vector2f vel1 = players[i].getVelocity();
                sf::Vector2f vel2 = players[j].getVelocity();
                
                // Calculate velocities along collision normal
                float vel1Normal = vel1.x * nx + vel1.y * ny;
                float vel2Normal = vel2.x * nx + vel2.y * ny;
                
                // Calculate velocities perpendicular to collision normal (unchanged)
                float vel1Tangent = vel1.x * (-ny) + vel1.y * nx;
                float vel2Tangent = vel2.x * (-ny) + vel2.y * nx;
                
                // Only resolve if objects are moving toward each other
                if(vel1Normal - vel2Normal <= 0) continue;
                
                // Conservation of momentum + restitution
                // Formula: v'n = ((m1*v1n + m2*v2n) ± restitution*m2*(v2n-v1n)) / (m1 + m2)
                float totalMass = mass1 + mass2;
                
                float newVel1Normal = ((mass1 * vel1Normal + mass2 * vel2Normal) + 
                                      mass2 * RESTITUTION * (vel2Normal - vel1Normal)) / totalMass;
                float newVel2Normal = ((mass1 * vel1Normal + mass2 * vel2Normal) + 
                                      mass1 * RESTITUTION * (vel1Normal - vel2Normal)) / totalMass;
                
                // Reconstruct velocities from normal and tangent components
                sf::Vector2f newVel1(
                    newVel1Normal * nx + vel1Tangent * (-ny),
                    newVel1Normal * ny + vel1Tangent * nx
                );
                sf::Vector2f newVel2(
                    newVel2Normal * nx + vel2Tangent * (-ny),
                    newVel2Normal * ny + vel2Tangent * nx
                );

                // Apply boosted impulses for more dramatic knockback
                const float impulseBoost = 1.15f;
                sf::Vector2f impulse1 = (newVel1 - vel1) * impulseBoost;
                sf::Vector2f impulse2 = (newVel2 - vel2) * impulseBoost;

                players[i].addVelocity(impulse1);
                players[j].addVelocity(impulse2);
            }
        }
    }
}

