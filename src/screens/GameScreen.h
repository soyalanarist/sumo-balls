#pragma once

#include "../core/Screen.h"
#include "menus/MenuAction.h"
#include "../simulation/Simulation.h"
#include "../game/controllers/HumanController.h"
#include "../game/controllers/AIController.h"
#include <memory>
#include <vector>

// GameScreen with singleplayer support
class GameScreen : public Screen {
public:
    explicit GameScreen(bool singlePlayer = false);
    ~GameScreen() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }
    MenuAction getMenuAction() const override { return action; }
    void resetMenuAction() override { action = MenuAction::NONE; }

private:
    bool singleplayer = false;
    MenuAction action = MenuAction::NONE;
    
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
    
    // Rendering
    float cameraZoom = 1.0f;
    
    void initializeSingleplayerGame();
    void updateGameLogic(float dt);
    void renderGameView();
    void renderUI();
    void handleGameEnd();
};
