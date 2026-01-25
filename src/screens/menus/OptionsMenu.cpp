#include "OptionsMenu.h"
#include "../../core/Settings.h"
#include <imgui.h>

OptionsMenu::OptionsMenu(bool isOverlay) : overlay(isOverlay) {}

void OptionsMenu::update() {
    // No background logic yet
}

void OptionsMenu::render() {
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Display");
    if (ImGui::Button("Windowed")) {
        action = MenuAction::SET_WINDOWED;
    }
    ImGui::SameLine();
    if (ImGui::Button("Fullscreen")) {
        action = MenuAction::SET_FULLSCREEN;
    }

    ImGui::Separator();
    ImGui::Text("Back");
    if (ImGui::Button(overlay ? "Resume" : "Main Menu")) {
        action = overlay ? MenuAction::RESUME : MenuAction::MAIN_MENU;
    }
    ImGui::End();
}
