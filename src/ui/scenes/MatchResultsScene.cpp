#include "MatchResultsScene.h"
#include "core/ScreenTransition.h"
#include <imgui.h>

MatchResultsScene::MatchResultsScene() = default;

void MatchResultsScene::update() {
    // No background logic yet
}

void MatchResultsScene::render() {
    ImGui::Begin("Match Ended", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%s", winnerText.empty() ? "Match Ended" : winnerText.c_str());
    if (ImGui::Button("Main Menu")) {
        returnToMenu = true;
    }
    ImGui::End();
}

ScreenTransition MatchResultsScene::getTransition() const {
    return returnToMenu ? ScreenTransition::TO_MAIN_MENU : ScreenTransition::NONE;
}

void MatchResultsScene::resetTransition() { returnToMenu = false; }
