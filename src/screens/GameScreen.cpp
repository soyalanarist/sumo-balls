#include "GameScreen.h"
#include "../ui/UIComponents.h"
#include "../core/Settings.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>

GameScreen::GameScreen(bool singlePlayer)
    : singleplayer(singlePlayer) {
    if (singleplayer) {
        initializeSingleplayerGame();
    }
}

GameScreen::~GameScreen() = default;

void GameScreen::initializeSingleplayerGame() {
    // Create simulation with arena
    const float ARENA_RADIUS = 300.0f;
    const Vec2 ARENA_CENTER = Vec2(600.0f, 400.0f);
    simulation = std::make_unique<Simulation>(ARENA_RADIUS, ARENA_CENTER);
    
    // Add player (ID 1) at random position in arena
    float playerAngle = (rand() % 360) * 3.14159f / 180.0f;
    Vec2 playerSpawn = ARENA_CENTER + Vec2(std::cos(playerAngle) * 150.0f, std::sin(playerAngle) * 150.0f);
    simulation->addPlayer(playerId, playerSpawn);
    
    // Create human controller for player
    playerController = std::make_unique<HumanController>();
    
    // Add 5 AI opponents
    const int NUM_AI = 5;
    for (int i = 0; i < NUM_AI; i++) {
        uint32_t aiId = playerId + 1 + i;
        float difficulty = 0.3f + (i * 0.15f); // Increasing difficulty
        
        // Spawn AI at random positions around arena
        float aiAngle = (rand() % 360) * 3.14159f / 180.0f;
        Vec2 aiSpawn = ARENA_CENTER + Vec2(std::cos(aiAngle) * 150.0f, std::sin(aiAngle) * 150.0f);
        simulation->addPlayer(aiId, aiSpawn);
        
        // Create AI controller
        auto ai = std::make_unique<AIController>(difficulty);
        aiControllers.push_back(std::move(ai));
    }
    
    gameRunning = true;
    gameEnded = false;
    gameTime = 0.0f;
    survivorCount = 1 + NUM_AI; // Player + 5 AI
    
    std::cout << "[GameScreen] Singleplayer game initialized with player and 5 AI opponents" << std::endl;
}

void GameScreen::update() {
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

void GameScreen::updateGameLogic(float dt) {
    if (!simulation) return;
    
    // Update game time
    gameTime += dt;
    
    // Check if game time expired
    if (gameTime >= GAME_DURATION) {
        gameEnded = true;
        handleGameEnd();
        return;
    }
    
    // Get current player snapshot
    auto players = simulation->snapshotPlayers();
    std::vector<Vec2> otherPlayerPositions;
    
    bool playerAlive = false;
    for (const auto& p : players) {
        if (p.id == playerId) {
            playerAlive = p.alive;
        } else {
            if (p.alive) {
                otherPlayerPositions.push_back(p.position);
            }
        }
    }
    
    // Update survivor count
    survivorCount = 0;
    for (const auto& p : players) {
        if (p.alive) survivorCount++;
    }
    
    // If player is dead, end game
    if (!playerAlive) {
        gameEnded = true;
        handleGameEnd();
        return;
    }
    
    // Get player position for controllers
    Vec2 playerPosition = Vec2(0, 0);
    for (const auto& p : players) {
        if (p.id == playerId) {
            playerPosition = p.position;
            break;
        }
    }
    
    // Get player input and apply
    Vec2 playerDirection = playerController->getMovementDirection(dt, playerPosition, otherPlayerPositions, 
                                                                   simulation->arenaCenter, simulation->arenaRadius);
    simulation->applyInput(playerId, playerDirection);
    
    // Update AI players
    int aiIndex = 0;
    for (int i = 0; i < 5; i++) {
        uint32_t aiId = playerId + 1 + i;
        
        // Find AI player position
        Vec2 aiPosition = Vec2(0, 0);
        bool aiAlive = false;
        for (const auto& p : players) {
            if (p.id == aiId) {
                aiPosition = p.position;
                aiAlive = p.alive;
                break;
            }
        }
        
        if (aiAlive && aiIndex < (int)aiControllers.size()) {
            Vec2 aiDirection = aiControllers[aiIndex]->getMovementDirection(
                dt, aiPosition, otherPlayerPositions,
                simulation->arenaCenter, simulation->arenaRadius
            );
            simulation->applyInput(aiId, aiDirection);
        }
        aiIndex++;
    }
    
    // Step simulation
    simulation->tick(dt);
}

void GameScreen::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##GameScreen", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    
    if (singleplayer) {
        renderGameView();
        renderUI();
    } else {
        ImGui::Text("Multiplayer game (stub)");
        if (ImGui::Button("Return to Main Menu")) {
            action = MenuAction::MAIN_MENU;
        }
    }
    
    ImGui::End();
}

void GameScreen::renderGameView() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Draw arena background
    ImVec2 arena_center = ImVec2(window_size.x * 0.5f, window_size.y * 0.65f);
    float arena_pixel_radius = 200.0f; // Scale arena for display
    
    ImU32 arena_bg_color = ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    draw_list->AddCircleFilled(arena_center, arena_pixel_radius, arena_bg_color);
    
    ImU32 arena_border_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    draw_list->AddCircle(arena_center, arena_pixel_radius, arena_border_color, 32, 3.0f);
    
    // Draw players if simulation exists
    if (simulation) {
        auto players = simulation->snapshotPlayers();
        const Vec2& simArenaCenter = simulation->arenaCenter;
        float simArenaRadius = simulation->getArenaRadius();
        
        for (const auto& player : players) {
            if (!player.alive) continue;
            
            // Convert simulation coords to screen coords
            float relX = (player.position.x - simArenaCenter.x) / simArenaRadius;
            float relY = (player.position.y - simArenaCenter.y) / simArenaRadius;
            
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
            
            draw_list->AddCircleFilled(screen_pos, 10.0f, player_color, 16);
            draw_list->AddCircle(screen_pos, 10.0f, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.5f)), 16, 1.0f);
        }
    }
}

void GameScreen::renderUI() {
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
        bool playerSurvived = survivorCount == 1; // Only player left
        if (playerSurvived) {
            UIComponents::CenteredHeading("🏆 YOU WIN! 🏆");
        } else {
            UIComponents::CenteredHeading("💔 DEFEATED 💔");
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
            action = MenuAction::MAIN_MENU;
        }
        
        ImGui::End();
    }
    
    // Control hints
    ImGui::SetCursorPos(ImVec2(20.0f, window_height - 50.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 0.6f));
    ImGui::TextUnformatted("Use WASD or Arrow Keys to move | ESC for Main Menu");
    ImGui::PopStyleColor();
}

void GameScreen::handleGameEnd() {
    // Game end logic placeholder
    // The game over screen is rendered in renderUI()
}

