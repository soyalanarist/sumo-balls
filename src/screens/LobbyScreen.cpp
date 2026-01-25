// Lobby screen stub during UI migration

#include "LobbyScreen.h"
#include "../ui/UIComponents.h"
#include <imgui.h>

LobbyScreen::LobbyScreen() = default;
LobbyScreen::~LobbyScreen() = default;

void LobbyScreen::update() {
    // TODO: Implement lobby flow with ImGui
}

void LobbyScreen::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##Lobbies", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Decorative top bar
    ImU32 accent_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + 4), accent_color);
    
    ImGui::Spacing();
    ImGui::SetCursorPosX(20);
    
    // Title
    UIComponents::CenteredHeading("🎮  Game Lobbies");
    ImGui::SetCursorPosX(20);
    ImGui::Spacing();
    
    // Search bar
    static char search_buf[128] = "";
    ImGui::SetCursorPosX(20);
    ImGui::SetNextItemWidth(ImGui::GetIO().DisplaySize.x - 40);
    ImGui::InputTextWithHint("##search", "🔍 Search lobbies...", search_buf, IM_ARRAYSIZE(search_buf));
    
    ImGui::SetCursorPosX(20);
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Lobbies section
    UIComponents::SectionHeader("Available Lobbies");
    ImGui::SetCursorPosX(20);
    
    // Placeholder lobby entries
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.20f, 1.0f));
    for (int i = 0; i < 3; i++) {
        ImGui::BeginChild(("LobbyItem##" + std::to_string(i)).c_str(), ImVec2(ImGui::GetIO().DisplaySize.x - 60, 80), true);
        
        ImGui::Text("Lobby #%d", i + 1);
        ImGui::SameLine(200);
        ImGui::TextColored(ImVec4(0.40f, 0.90f, 0.40f, 1.0f), "● Online");
        ImGui::SameLine(300);
        ImGui::Text("Players: %d/4", i + 1);
        
        ImGui::TextDisabled("Mode: Battle Royale | Map: Arena 1");
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        if (ImGui::Button(("Join##" + std::to_string(i)).c_str(), ImVec2(100, 30))) {
            // Join lobby
        }
        
        ImGui::EndChild();
        ImGui::Spacing();
    }
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPosX(20);
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Footer buttons
    UIComponents::HorizontalSeparator(1.5f);
    const float button_width = 200.0f;
    const float button_spacing = 20.0f;
    float button_x = ImGui::GetIO().DisplaySize.x * 0.5f - button_width - button_spacing * 0.5f;
    
    ImGui::SetCursorPosX(button_x);
    if (UIComponents::PrimaryButton("➕ Create Lobby", ImVec2(button_width, 40))) {
        // Create new lobby
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(button_x + button_width + button_spacing);
    if (UIComponents::SecondaryButton("Back", ImVec2(button_width, 40))) {
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
