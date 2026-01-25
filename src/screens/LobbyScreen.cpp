// Lobby screen stub during UI migration

#include "LobbyScreen.h"
#include <imgui.h>

LobbyScreen::LobbyScreen() = default;
LobbyScreen::~LobbyScreen() = default;

void LobbyScreen::update() {
    // TODO: Implement lobby flow with ImGui
}

void LobbyScreen::render() {
    ImGui::Begin("Lobbies", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Lobbies (stub)");
    ImGui::Separator();
    ImGui::TextWrapped("Browse, create, and join lobbies will be here.");
    if (ImGui::Button("Back to Main Menu")) {
        action = MenuAction::MAIN_MENU;
    }
    ImGui::End();
}

MenuAction LobbyScreen::getMenuAction() const {
    return action;
}

void LobbyScreen::resetMenuAction() {
    action = MenuAction::NONE;
}
