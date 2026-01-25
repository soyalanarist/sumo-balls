#include "GameScreen.h"
#include <imgui.h>

GameScreen::GameScreen(bool singlePlayer)
    : singleplayer(singlePlayer) {}

GameScreen::~GameScreen() = default;

void GameScreen::update() {
    // Gameplay removed during migration; keep stub alive
}

void GameScreen::render() {
    ImGui::Begin("Game (stub)");
    ImGui::Text("Gameplay removed during SDL migration");
    if (ImGui::Button("Return to Main Menu")) {
        action = MenuAction::MAIN_MENU;
    }
    ImGui::End();
}

