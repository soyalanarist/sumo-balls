#include "MatchScene.h"
#include "ui/components/UIComponents.h"
#include "core/Settings.h"
#include "core/KeyBindings.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>

bool MatchScene::handleInput(const SDL_Event& event) {
    // Handle ESC key for pause during active gameplay (any game mode)
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        // Pause any active, non-ended game regardless of game mode
        if (gameRunning && !gameEnded && !paused) {
            action = ScreenTransition::PAUSE;
            return true;  // Event consumed - prevents ESC from quitting
        }
    }
    return false;  // Event not handled, pass to other screens
}

MatchScene::MatchScene(bool singlePlayer)
    : singleplayer(singlePlayer) {
    if (singleplayer) {
        initializeSingleplayerGame();
    }
}

MatchScene::~MatchScene() = default;

void MatchScene::initializeSingleplayerGame() {
    // Create simulation with arena
    const float ARENA_RADIUS = 650.0f;
    const Vec2 ARENA_CENTER = Vec2(640.0f, 420.0f);
    simulation = std::make_unique<Simulation>(ARENA_RADIUS, ARENA_CENTER);
    
    // Randomize starting angle offset for variety
    float angleOffset = (rand() % 360) * 3.14159f / 180.0f;
    
    // Add player (ID 1) at position 0 (evenly spaced around circle)
    const float SPAWN_RADIUS = ARENA_RADIUS * 0.55f;
    const int TOTAL_PLAYERS = 6;  // 1 human + 5 AI
    const float ANGLE_SPACING = 2.0f * 3.14159f / TOTAL_PLAYERS;  // 60 degrees
    
    float playerAngle = angleOffset;
    Vec2 playerSpawn = ARENA_CENTER + Vec2(std::cos(playerAngle) * SPAWN_RADIUS, std::sin(playerAngle) * SPAWN_RADIUS);
    simulation->addPlayer(playerId, playerSpawn);
    
    // Create human controller for player
    playerController = std::make_unique<HumanController>();
    
    // Add 5 AI opponents with difficulty from Settings, evenly spaced
    const int NUM_AI = 5;
    DifficultyLevel selectedLevel = static_cast<DifficultyLevel>(Settings::aiDifficulty);
    for (int i = 0; i < NUM_AI; i++) {
        uint32_t aiId = playerId + 1 + i;
        float aiAngle = angleOffset + ANGLE_SPACING * (i + 1);
        Vec2 aiSpawn = ARENA_CENTER + Vec2(std::cos(aiAngle) * SPAWN_RADIUS, std::sin(aiAngle) * SPAWN_RADIUS);
        simulation->addPlayer(aiId, aiSpawn);
        auto ai = std::make_unique<AIController>(selectedLevel);
        aiControllers.push_back(std::move(ai));
    }
    
    gameRunning = true;
    gameEnded = false;
    paused = false;
    countdownActive = true;
    countdownTime = 3.0f;
    gameTime = 0.0f;
    survivorCount = 1 + NUM_AI; // Player + 5 AI
    playerWon = false;
    prevAlive.clear();
    prevAlive[playerId] = true;
    for (int i = 0; i < NUM_AI; ++i) prevAlive[playerId + 1 + i] = true;
    exitAnims.clear();
    
    std::cout << "[GameScreen] Singleplayer game initialized with player and 5 AI opponents" << std::endl;
}

void MatchScene::update() {
    if (!singleplayer || !gameRunning || gameEnded) {
        return;
    }
    
    // Get frame time (use fixed 60 FPS for consistency)
    static uint32_t lastTime = SDL_GetTicks();
    uint32_t currentTime = SDL_GetTicks();
    float dt = std::min(0.033f, (currentTime - lastTime) / 1000.0f);
    lastTime = currentTime;
    
    updateGameLogic(dt);
}

void MatchScene::updateGameLogic(float dt) {
    if (!simulation || paused) return;
    if (countdownActive) {
        countdownTime -= dt;
        if (countdownTime <= 0.0f) {
            countdownTime = 0.0f;
            countdownActive = false;
        } else {
            return; // wait until countdown finishes
        }
    }
    
    // Update arena shrinking
    simulation->updateArenaShrink(dt);
    
    // Update game time
    gameTime += dt;
    
    // Time limit disabled: play until one player remains
    
    // Get current player snapshot
    auto players = simulation->snapshotPlayers();
    std::vector<std::pair<Vec2, Vec2>> otherPlayerStates; // pos, vel
    
    bool playerAlive = false;
    Vec2 playerVelocity{0.f, 0.f};
    for (const auto& p : players) {
        if (p.id == playerId) {
            playerAlive = p.alive;
            playerVelocity = p.velocity;
        } else {
            if (p.alive) {
                otherPlayerStates.push_back({p.position, p.velocity});
            }
        }
    }
    
    // Update survivor count
    survivorCount = 0;
    for (const auto& p : players) {
        if (p.alive) survivorCount++;
    }

    // Trigger elimination animations for newly dead players
    updateExitAnimations(dt, players);
    
    // Get player position for controllers (if still alive)
    Vec2 playerPosition = Vec2(0, 0);
    for (const auto& p : players) {
        if (p.id == playerId) {
            playerPosition = p.position;
            break;
        }
    }
    
    // Get player input and apply (only if player is alive)
    if (playerAlive) {
        Vec2 playerDirection = playerController->getMovementDirection(
            dt, playerPosition, playerVelocity, otherPlayerStates,
            simulation->arenaCenter, simulation->getCurrentArenaRadius(), simulation->getArenaAge());
        simulation->applyInput(playerId, playerDirection);
    }
    
    // Update AI players
    int aiIndex = 0;
    for (int i = 0; i < 5; i++) {
        uint32_t aiId = playerId + 1 + i;
        
        // Find AI player position and velocity, and build opponents list excluding this AI
        Vec2 aiPosition = Vec2(0, 0);
        Vec2 aiVelocity = Vec2(0, 0);
        bool aiAlive = false;
        std::vector<std::pair<Vec2, Vec2>> aiOpponents;
        aiOpponents.reserve(otherPlayerStates.size());
        for (const auto& p : players) {
            if (!p.alive) continue;
            if (p.id == aiId) {
                aiPosition = p.position;
                aiVelocity = p.velocity;
                aiAlive = true;
            } else {
                aiOpponents.push_back({p.position, p.velocity});
            }
        }
        
        if (aiAlive && aiIndex < (int)aiControllers.size()) {
            Vec2 aiDirection = aiControllers[aiIndex]->getMovementDirection(
                dt, aiPosition, aiVelocity, aiOpponents,
                simulation->arenaCenter, simulation->getCurrentArenaRadius(), simulation->getArenaAge()
            );
            simulation->applyInput(aiId, aiDirection);
        }
        aiIndex++;
    }
    
    // Step simulation
    simulation->tick(dt);

    // Recalculate survivors and player alive after physics step
    auto postPlayers = simulation->snapshotPlayers();
    survivorCount = 0;
    bool postPlayerAlive = false;
    aiNearEdge = 0;
    aiAvgEdgeDist = 0.0f;
    float currentR = simulation->getCurrentArenaRadius();
    int aiCount = 0;
    for (const auto& p : postPlayers) {
        if (p.alive) {
            survivorCount++;
            if (p.id == playerId) postPlayerAlive = true;
            else {
                aiCount++;
                float dist = VectorMath::magnitude(p.position - simulation->arenaCenter);
                float edgeDist = currentR - dist;
                aiAvgEdgeDist += edgeDist;
                if (edgeDist < 80.f) aiNearEdge++;
            }
        }
    }
    if (aiCount > 0) aiAvgEdgeDist /= aiCount;

    // End when only one survivor remains, and determine winner
    if (survivorCount == 1) {
        playerWon = postPlayerAlive;
        gameEnded = true;
        handleGameEnd();
        return;
    }
}

void MatchScene::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##GameScreen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    
    if (singleplayer) {
        renderGameView();
        renderUI();
    } else {
        ImGui::Text("Multiplayer game (stub)");
        if (ImGui::Button("Return to Main Menu")) {
            action = ScreenTransition::TO_MAIN_MENU;
        }
    }
    
    ImGui::End();
}

void MatchScene::renderGameView() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Draw arena background
    ImVec2 arena_center = ImVec2(window_size.x * 0.5f, window_size.y * 0.62f);
    float arena_pixel_radius = 300.0f; // Display scale
    
    // Get current arena state
    float simArenaRadius = 500.0f;  // Initial radius
    float currentSimRadius = simArenaRadius;
    if (simulation) {
        simArenaRadius = simulation->arenaRadius;
        currentSimRadius = simulation->getCurrentArenaRadius();
    }
    
    // Calculate current arena display radius
    float currentDisplayRadius = (currentSimRadius / simArenaRadius) * arena_pixel_radius;
    
    // Draw current safe zone background
    ImU32 arena_bg_color = ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    draw_list->AddCircleFilled(arena_center, currentDisplayRadius, arena_bg_color);
    
    // Draw current arena boundary (bright, shows actual playable area)
    ImU32 arena_border_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    draw_list->AddCircle(arena_center, currentDisplayRadius, arena_border_color, 32, 3.0f);
    
    // Draw players if simulation exists
    if (simulation) {
        auto players = simulation->snapshotPlayers();
        const Vec2& simArenaCenter = simulation->arenaCenter;
        float simRadius = simulation->arenaRadius;  // Use initial radius for coordinate mapping
        float drawScale = (simRadius > 0.01f) ? (arena_pixel_radius / simRadius) : 0.6f;
        float drawPlayerRadius = simulation->getPlayerRadius() * drawScale;
        
        for (const auto& player : players) {
            if (!player.alive) continue;
            
            // Convert simulation coords to screen coords
            float relX = (player.position.x - simArenaCenter.x) / simRadius;
            float relY = (player.position.y - simArenaCenter.y) / simRadius;
            
            ImVec2 screen_pos = ImVec2(
                arena_center.x + relX * arena_pixel_radius,
                arena_center.y + relY * arena_pixel_radius
            );
            
            // Draw player as circle
            ImU32 player_color;
            if (player.id == playerId) {
                // Player is blue
                player_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
            } else {
                // AI are red/orange
                player_color = ImGui::GetColorU32(ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
            }
            
            float pr = drawPlayerRadius;
            draw_list->AddCircleFilled(screen_pos, pr, player_color, 20);
            draw_list->AddCircle(screen_pos, pr, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)), 20, 1.4f);
        }

        // Draw exit animations (expanding fade rings at elimination positions)
        for (auto& e : exitAnims) {
            // Convert sim coords to screen
            float relX = (e.pos.x - simArenaCenter.x) / simRadius;
            float relY = (e.pos.y - simArenaCenter.y) / simRadius;
            ImVec2 ep = ImVec2(arena_center.x + relX * arena_pixel_radius,
                               arena_center.y + relY * arena_pixel_radius);
            float t = e.t; // 0..1
            float radius = 14.0f + 28.0f * t;
            float alpha = 1.0f - t;
            ImU32 col = ImGui::GetColorU32(ImVec4(1.0f, 0.5f, 0.2f, alpha));
            draw_list->AddCircle(ep, radius, col, 28, 3.0f);
        }
    }
}

void MatchScene::renderUI() {
    ImGuiIO& io = ImGui::GetIO();
    float window_width = ImGui::GetWindowWidth();
    float window_height = ImGui::GetWindowHeight();
    
    // Top HUD - Game info
    ImGui::SetCursorPos(ImVec2(20.0f, 20.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::Text("TIME: %.1fs / %.0fs", gameTime, GAME_DURATION);
    ImGui::PopStyleColor();
    
    // Survivors counter
    ImGui::SetCursorPos(ImVec2(window_width - 250.0f, 20.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
    ImGui::Text("SURVIVORS: %u", survivorCount);
    ImGui::PopStyleColor();

    // Arena shrink status + AI telemetry
    if (simulation) {
        ImGui::SetCursorPos(ImVec2(window_width - 250.0f, 45.0f));
        float currentR = simulation->getCurrentArenaRadius();
        float shrinkRate = 7.0f; // matches Simulation
        ImGui::Text("ARENA: %.0f (\u2193 %.1f/s)", currentR, shrinkRate);

        ImGui::SetCursorPos(ImVec2(window_width - 250.0f, 65.0f));
        ImGui::TextDisabled("AI near edge: %d | avg edge: %.1f", aiNearEdge, aiAvgEdgeDist);
    }
    
    // Game Over screen
    if (gameEnded) {
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImVec2 center = vp ? vp->GetCenter() : ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_FirstUseEver);
        
        ImGui::Begin("##GameOver", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        
        // Decorative bar
        ImU32 accent_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + 4), accent_color);
        
        ImGui::Spacing();
        
        // Title
        bool playerSurvived = playerWon;
        if (playerSurvived) {
            UIComponents::CenteredHeading("YOU WIN!");
        } else {
            UIComponents::CenteredHeading("DEFEATED");
        }
        
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        ImGui::TextUnformatted("===========================");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        ImGui::Text("Final Time: %.1f seconds", gameTime);
        ImGui::Text("Survivors: %u", survivorCount);
        ImGui::Text("Opponents Defeated: %u", (6 - survivorCount));
        
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Buttons
        const float button_width = 200.0f;
        const float button_spacing = 20.0f;
        float button_x = (ImGui::GetWindowWidth() - (button_width * 2 + button_spacing)) * 0.5f;
        
        ImGui::SetCursorPosX(button_x);
        if (UIComponents::PrimaryButton("Play Again", ImVec2(button_width, 40))) {
            // Restart game
            initializeSingleplayerGame();
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(button_x + button_width + button_spacing);
        if (UIComponents::SecondaryButton("Main Menu", ImVec2(button_width, 40))) {
            action = ScreenTransition::TO_MAIN_MENU;
        }
        
        ImGui::End();
    }
    
    // Control hints (reflect current keybindings)
    ImGui::SetCursorPos(ImVec2(20.0f, window_height - 50.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 0.6f));
    bool lefty = Settings::leftyMode;
    SDL_Scancode up = KeyBindings::getMoveUpKey(lefty);
    SDL_Scancode left = KeyBindings::getMoveLeftKey(lefty);
    SDL_Scancode down = KeyBindings::getMoveDownKey(lefty);
    SDL_Scancode right = KeyBindings::getMoveRightKey(lefty);
    std::string moveKeys = std::string(KeyBindings::getKeyName(up)) +
                           std::string(KeyBindings::getKeyName(left)) +
                           std::string(KeyBindings::getKeyName(down)) +
                           std::string(KeyBindings::getKeyName(right));
    std::string hint = "Use " + moveKeys + " to move | ESC to pause";
    ImGui::TextUnformatted(hint.c_str());
    ImGui::PopStyleColor();

    // Countdown overlay
    if (countdownActive) {
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        ImVec2 vp = ImGui::GetMainViewport()->Pos;
        ImVec2 vs = ImGui::GetMainViewport()->Size;
        dl->AddRectFilled(vp, ImVec2(vp.x + vs.x, vp.y + vs.y), ImGui::GetColorU32(ImVec4(0, 0, 0, 0.35f)));

        int number = static_cast<int>(std::ceil(countdownTime));
        float frac = countdownTime - std::floor(countdownTime);
        float alpha = 0.2f + 0.8f * (1.0f - frac); // fade per number

        ImGui::SetNextWindowPos(ImVec2(vp.x + vs.x * 0.5f, vp.y + vs.y * 0.45f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::Begin("##CountdownOverlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%d", number);
        ImGui::End();
        ImGui::PopFont();
        ImGui::PopStyleColor();
    }
}

void MatchScene::handleGameEnd() {
    // Game end logic placeholder
    // The game over screen is rendered in renderUI()
}

void MatchScene::updateExitAnimations(float dt, const std::vector<SimSnapshotPlayer>& players) {
    // Detect freshly eliminated players and enqueue animation with position
    for (const auto& p : players) {
        bool wasAlive = prevAlive[p.id];
        if (wasAlive && !p.alive) {
            ExitAnim e{p.id, p.position, 0.f};
            exitAnims.push_back(e);
            prevAlive[p.id] = false;
        } else if (!wasAlive && p.alive) {
            // respawn case (not expected in singleplayer); update state
            prevAlive[p.id] = true;
        }
    }
    // Advance animations and remove finished
    for (auto& e : exitAnims) {
        e.t += dt / 0.9f; // ~0.9s animation duration
        if (e.t > 1.f) e.t = 1.f;
    }
    exitAnims.erase(std::remove_if(exitAnims.begin(), exitAnims.end(), [](const ExitAnim& e){ return e.t >= 1.f; }), exitAnims.end());
}

