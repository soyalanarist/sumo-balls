#include "GameScreen.h"
#include "menus/GameOverMenu.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include "../core/Settings.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>

GameScreen::GameScreen():
    arena({600.f, 450.f}, 300.f),
    initialArenaRadius(300.f){
    
    // Load font once at initialization
    if(!font.loadFromFile("assets/arial.ttf")) {
        throw std::runtime_error("Failed to load font: assets/arial.ttf");
    }

    initOnlineConfigFromEnv();

    if(onlineMode) {
        // Online: players will be created from server snapshots; no local AI spawn
        countdownActive = false;  // Disable countdown in online mode
        return;
    }
    
    // Calculate 6 equidistant spawn positions around arena center
    sf::Vector2f arenaCenter = arena.center;
    float spawnRadius = 200.f;  // Distance from center
    std::vector<sf::Vector2f> spawnPositions;
    
    for(int i = 0; i < 6; i++) {
        float angle = (i * 6.2831853f) / 6.f;  // 6 equally-spaced angles (60° apart)
        float x = arenaCenter.x + spawnRadius * std::cos(angle);
        float y = arenaCenter.y + spawnRadius * std::sin(angle);
        spawnPositions.emplace_back(x, y);
    }
    
    // Randomize which position each player gets
    std::random_device rd;
    std::mt19937 rng(rd());
    std::vector<int> indices = {0, 1, 2, 3, 4, 5};
    std::shuffle(indices.begin(), indices.end(), rng);
    
    // Human player with selected color at randomized position
    auto humanCtrl = std::make_unique<HumanController>();
    players.emplace_back(spawnPositions[indices[0]], std::move(humanCtrl), Settings::getPlayerColor());
    
    // 5 AI players with randomized positions and varied difficulties
    auto ai1 = std::make_unique<AIController>(0.5f);  // 50% difficulty
    players.emplace_back(spawnPositions[indices[1]], std::move(ai1));
    
    auto ai2 = std::make_unique<AIController>(0.6f);  // 60% difficulty
    players.emplace_back(spawnPositions[indices[2]], std::move(ai2));
    
    auto ai3 = std::make_unique<AIController>(0.7f);  // 70% difficulty
    players.emplace_back(spawnPositions[indices[3]], std::move(ai3));
    
    auto ai4 = std::make_unique<AIController>(0.65f);  // 65% difficulty
    players.emplace_back(spawnPositions[indices[4]], std::move(ai4));
    
    auto ai5 = std::make_unique<AIController>(0.55f);  // 55% difficulty
    players.emplace_back(spawnPositions[indices[5]], std::move(ai5));
}

void GameScreen::initOnlineConfigFromEnv() {
    onlineMode = Settings::onlineEnabled;
    netHost = Settings::onlineHost;
    netPort = static_cast<std::uint16_t>(Settings::onlinePort);

    if (const char* envOnline = std::getenv("SUMO_ONLINE")) {
        std::string v(envOnline);
        std::transform(v.begin(), v.end(), v.begin(), ::tolower);
        onlineMode = (v == "1" || v == "true" || v == "yes");
    }
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

void GameScreen::update(sf::Time dt, [[maybe_unused]] sf::RenderWindow& window) {
    try{
        frameCount++;
        
        // Online client mode: process networking FIRST, before countdown logic
        if(onlineMode) {
            if(!netConnected) {
                static bool connectionAttempted = false;
                if (!connectionAttempted) {
                    if(netClient.connect(netHost, netPort)) {
                        connectionAttempted = true;
                    }
                }
            }

            handleNetService();
            sendNetInput(dt.asSeconds());
            sendNetPing(dt.asSeconds());

            interpolateSnapshots();
            return;
        }
        
        // Handle countdown before game starts (offline mode only)
        if(countdownActive) {
            countdownTime -= dt.asSeconds();
            if(countdownTime <= 0.f) {
                countdownActive = false;
                countdownTime = 0.f;
            }
            // During countdown, don't update game logic
            return;
        }
        
        // Increment game time only if game is not over
        if(!gameOver) {
            gameTime += dt.asSeconds();
        }
        
        // Check for pause key (P)
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            menuAction = MenuAction::PAUSE;
            return;  // Don't update game state if pausing
        }
        
        menuAction = MenuAction::NONE;  // Clear any pending actions
        positions.clear();
        positions.reserve(players.size());
        for(auto& p : players){
            if(p.isAlive()){
                positions.push_back(p.getPosition());
            }
        }

        for(auto& p : players){
            float speedMult = getSpeedMultiplier();
            p.update(dt.asSeconds(), positions, arena.center, arena.radius, speedMult);
        }
        
        resolvePlayerCollisions();
        
        // Shrink arena over time (continuous shrinking until game ends)
        if(!gameOver) {
            float shrinkRate = 5.0f;  // pixels per second
            float newRadius = 300.f - (gameTime * shrinkRate);
            if(newRadius > 0.f) {
                arena.setRadius(newRadius);
            }
        }
        
        // Check arena boundaries - mark players as dead if 50% outside arena
        for(auto& p : players){
            if(p.isAlive()) {
                sf::Vector2f playerPos = p.getPosition();
                float playerRadius = p.getRadius();
                
                // Distance from player center to arena center
                float dx = playerPos.x - arena.center.x;
                float dy = playerPos.y - arena.center.y;
                float distToCenter = std::sqrt(dx * dx + dy * dy);
                
                // Player is dead if their center is more than (arenaRadius + 50% of playerRadius) away
                float deathDistance = arena.radius + playerRadius * 0.5f;
                
                if(distToCenter > deathDistance) {
                    // Create explosion before marking dead
                    createExplosion(playerPos, p.getVelocity());
                    p.setAlive(false);
                }
            }
        }
        
        // Update particles
        updateParticles(dt.asSeconds());
        
        // Check if game is over (only one player alive)
        if(!gameOver){
            int aliveCount = 0;
            for(auto& p : players){
                if(p.isAlive()) aliveCount++;
            }
            
            if(aliveCount <= 1) {
                gameOver = true;
                gameOverTime = 0.f;  // Start game over timer
            }
        } else {
            gameOverTime += dt.asSeconds();  // Track time since game ended
        }
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
        if(onlineMode) {
            arena.render(window);
            for(auto& kv : netPlayers) {
                if(kv.second.isAlive()) {
                    kv.second.render(window);
                }
            }
            // HUD: show RTT/status
            sf::Text netInfo;
            netInfo.setFont(font);
            netInfo.setCharacterSize(18);
            netInfo.setFillColor(sf::Color::White);
            std::string rttStr = (rttMs >= 0.f) ? std::to_string(static_cast<int>(rttMs)) + " ms" : "--";
            netInfo.setString("Online  RTT: " + rttStr);
            netInfo.setPosition(10.f, 10.f);
            window.draw(netInfo);
            return;
        }

        arena.render(window);

        for (auto& playerEntity : players) {
            if (playerEntity.isAlive()) {
                playerEntity.render(window);
            }
        }
        
        // Render particles
        for(auto& particle : particles) {
            sf::CircleShape particleShape(particle.radius);
            
            // Calculate alpha fade (fade out as particle dies)
            float alphaRatio = particle.timeRemaining / particle.lifetime;
            int alpha = static_cast<int>(255.f * alphaRatio);
            
            particleShape.setFillColor(sf::Color(255, 200, 100, alpha));  // Orange/yellow particles
            particleShape.setPosition(particle.position.x - particle.radius, 
                                     particle.position.y - particle.radius);
            
            window.draw(particleShape);
        }
        
        // Countdown overlay before game starts
        if(countdownActive) {
            // Draw greyed out overlay
            sf::RectangleShape overlay({1200.f, 900.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            
            // Load font for countdown numbers
            // Determine countdown number (3, 2, 1)
            int countdownNumber = static_cast<int>(std::ceil(countdownTime));
            if(countdownNumber > 0 && countdownNumber <= 3) {
                sf::Text countdownText;
                countdownText.setFont(font);
                countdownText.setString(std::to_string(countdownNumber));
                countdownText.setCharacterSize(200);
                
                // Use player's color
                sf::Color playerColor = Settings::getPlayerColor();
                
                // Calculate fade effect (fade in and out within each second)
                float timeInSecond = countdownTime - std::floor(countdownTime);
                float fadeAlpha = 1.0f;
                
                // Fade in during first 0.2s, fade out during last 0.3s
                if(timeInSecond > 0.7f) {
                    fadeAlpha = (1.0f - timeInSecond) / 0.3f;  // Fade out
                } else if(timeInSecond < 0.2f) {
                    fadeAlpha = timeInSecond / 0.2f;  // Fade in
                }

                playerColor.a = static_cast<sf::Uint8>(255 * fadeAlpha);
                countdownText.setFillColor(playerColor);

                // Center the text
                sf::FloatRect textBounds = countdownText.getLocalBounds();
                countdownText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                       textBounds.top + textBounds.height / 2.0f);
                countdownText.setPosition(600.f, 400.f);
                window.draw(countdownText);
            }
        }
        
        // If game is over, show overlay message
        if(gameOver) {
            // Draw semi-transparent overlay
            sf::RectangleShape overlay({1200.f, 900.f});
            overlay.setFillColor(sf::Color(0, 0, 0, 100));
            window.draw(overlay);
            
            // Note: Menu will be handled by ScreenStack when we return to main menu
        }
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
    if(gameOver && gameOverTime >= 0.7f) {
        return MenuAction::MAIN_MENU;
    }
    
    return MenuAction::NONE;
}

void GameScreen::resetMenuAction() {
    menuAction = MenuAction::NONE;
}

void GameScreen::createExplosion(sf::Vector2f position, sf::Vector2f velocity) {
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
            PARTICLE_RADIUS
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

