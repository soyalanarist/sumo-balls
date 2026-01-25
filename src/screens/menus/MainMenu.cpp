#include "MainMenu.h"
#include <imgui.h>

MainMenu::MainMenu() {}

void MainMenu::update() {
    // ImGui handles all UI updates automatically
}

void MainMenu::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("MainMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    
    // Title
    ImGui::SetCursorPosY(40);
    float textWidth = ImGui::CalcTextSize("Sumo Balls").x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textWidth) * 0.5f);
    ImGui::Text("Sumo Balls");
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) * 0.5f);
    ImGui::SetCursorPosY(150);
    
    if (ImGui::Button("Singleplayer", ImVec2(300, 70))) {
        action = MenuAction::START_SINGLEPLAYER;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) * 0.5f);
    ImGui::SetCursorPosY(240);
    if (ImGui::Button("Multiplayer", ImVec2(300, 70))) {
        action = MenuAction::LOBBIES;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) * 0.5f);
    ImGui::SetCursorPosY(330);
    if (ImGui::Button("Friends", ImVec2(300, 70))) {
        action = MenuAction::FRIENDS;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) * 0.5f);
    ImGui::SetCursorPosY(420);
    if (ImGui::Button("Options", ImVec2(300, 70))) {
        action = MenuAction::OPTIONS;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 300) * 0.5f);
    ImGui::SetCursorPosY(510);
    if (ImGui::Button("Quit", ImVec2(300, 70))) {
        action = MenuAction::QUIT;
    }
    
    ImGui::End();
}
