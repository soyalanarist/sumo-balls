#include "GameOverMenu.h"
#include <imgui.h>

GameOverMenu::GameOverMenu() = default;

void GameOverMenu::update() {
    // No background logic yet
}

void GameOverMenu::render() {
    ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Game Over (stub)");
    if (ImGui::Button("Main Menu")) {
        action = MenuAction::MAIN_MENU;
    }
    ImGui::End();
}
