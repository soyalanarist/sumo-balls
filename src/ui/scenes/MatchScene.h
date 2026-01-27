#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include "game/simulation/Simulation.h"
#include "game/controllers/HumanController.h"
#include "game/controllers/AIController.h"
#include <memory>
#include <vector>

// Gameplay scene with singleplayer support
class MatchScene : public Screen {
public:
    explicit MatchScene(bool singlePlayer = false);
    ~MatchScene() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }
    bool handleInput(const SDL_Event& event) override;
    ScreenTransition getTransition() const override { return action; }
    void resetTransition() override { action = ScreenTransition::NONE; }
    void setPaused(bool p) { paused = p; }

private:
    bool singleplayer = false;
    ScreenTransition action = ScreenTransition::NONE;
    
    // Game state
    std::unique_ptr<Simulation> simulation;
    std::unique_ptr<HumanController> playerController;
    std::vector<std::unique_ptr<AIController>> aiControllers;
    
    // Game flow
    bool gameRunning = false;
    bool gameEnded = false;
    float gameTime = 0.0f;
    const float GAME_DURATION = 300.0f; // 5 minute match
    uint32_t playerId = 1;
    uint32_t survivorCount = 0;
    bool playerWon = false;
    std::unordered_map<uint32_t, bool> prevAlive;
    struct ExitAnim { uint32_t id; Vec2 pos; float t; };
    std::vector<ExitAnim> exitAnims;
    
    // Rendering
    float cameraZoom = 1.0f;

    bool paused = false;

    // Telemetry
    int aiNearEdge = 0;
    float aiAvgEdgeDist = 0.0f;

    // Countdown
    bool countdownActive = true;
    float countdownTime = 3.0f;
    
    void initializeSingleplayerGame();
    void updateGameLogic(float dt);
    void renderGameView();
    void renderUI();
    void handleGameEnd();
    void updateExitAnimations(float dt, const std::vector<SimSnapshotPlayer>& players);
};
