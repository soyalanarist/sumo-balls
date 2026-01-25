#include "GameEndedScreen.h"
#include "menus/MenuAction.h"
#include <imgui.h>

GameEndedScreen::GameEndedScreen() = default;

void GameEndedScreen::update() {
    // No background logic yet
}

void GameEndedScreen::render() {
    ImGui::Begin("Match Ended", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%s", winnerText.empty() ? "Match Ended" : winnerText.c_str());
    if (ImGui::Button("Main Menu")) {
        returnToMenu = true;
    }
    ImGui::End();
}

MenuAction GameEndedScreen::getMenuAction() const {
    return returnToMenu ? MenuAction::MAIN_MENU : MenuAction::NONE;
}

void GameEndedScreen::resetMenuAction() { returnToMenu = false; }
