#include "PauseMenu.h"
#include <imgui.h>

PauseMenu::PauseMenu() = default;

void PauseMenu::update(){
    // No background logic yet
}

void PauseMenu::render() {
    ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Game Paused");
    if (ImGui::Button("Resume")) {
        action = MenuAction::RESUME;
    }
    if (ImGui::Button("Options")) {
        action = MenuAction::OPTIONS;
    }
    if (ImGui::Button("Main Menu")) {
        action = MenuAction::MAIN_MENU;
    }
    if (ImGui::Button("Quit")) {
        action = MenuAction::QUIT;
    }
    ImGui::End();
}