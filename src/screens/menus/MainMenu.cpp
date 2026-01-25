#include "MainMenu.h"
#include <imgui.h>
#include <cmath>

MainMenu::MainMenu() {}

void MainMenu::update() {
    // ImGui handles all UI updates automatically
}

void MainMenu::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##MainMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float window_width = ImGui::GetWindowWidth();
    float window_height = ImGui::GetWindowHeight();
    
    // Decorative background pattern (subtle gradient lines)
    ImU32 line_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 0.08f));
    for (int i = 0; i < 5; i++) {
        float y = (window_height / 5) * i;
        draw_list->AddLine(ImVec2(0, y), ImVec2(window_width, y), line_color, 1.0f);
    }
    
    // Title section with decoration
    ImGui::SetCursorPosY(30);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font for now
    
    float title_width = ImGui::CalcTextSize("SUMO BALLS").x;
    ImGui::SetCursorPosX((window_width - title_width) * 0.5f);
    ImGui::TextUnformatted("SUMO BALLS");
    
    ImGui::PopFont();
    ImGui::PopStyleColor();
    
    // Subtitle
    ImGui::SetCursorPosY(70);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 0.8f));
    float subtitle_width = ImGui::CalcTextSize("Competitive Battle Arena").x;
    ImGui::SetCursorPosX((window_width - subtitle_width) * 0.5f);
    ImGui::TextUnformatted("Competitive Battle Arena");
    ImGui::PopStyleColor();
    
    // Menu buttons with better spacing and styling
    const float button_width = 280.0f;
    const float button_height = 60.0f;
    const float button_spacing = 20.0f;
    const float start_y = 150.0f;
    
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(start_y);
    
    // Push button style for all menu buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 14.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.68f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.48f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.97f, 1.0f));
    
    // Singleplayer Button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    if (ImGui::Button("🎮  Singleplayer", ImVec2(button_width, button_height))) {
        action = MenuAction::START_SINGLEPLAYER;
    }
    
    // Multiplayer Button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (ImGui::Button("👥  Multiplayer", ImVec2(button_width, button_height))) {
        action = MenuAction::LOBBIES;
    }
    
    // Friends Button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (ImGui::Button("🤝  Friends", ImVec2(button_width, button_height))) {
        action = MenuAction::FRIENDS;
    }
    
    // Options Button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (ImGui::Button("⚙️  Options", ImVec2(button_width, button_height))) {
        action = MenuAction::OPTIONS;
    }
    
    // Quit Button (different styling)
    ImGui::PopStyleColor(4);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.97f, 1.0f));
    
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing * 1.5f);
    if (ImGui::Button("❌  Quit", ImVec2(button_width, button_height))) {
        action = MenuAction::QUIT;
    }
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    
    // Footer info
    ImGui::SetCursorPosY(window_height - 50.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 0.6f));
    float footer_width = ImGui::CalcTextSize("v0.1.0 | Play Online").x;
    ImGui::SetCursorPosX((window_width - footer_width) * 0.5f);
    ImGui::TextUnformatted("v0.1.0 | Play Online");
    ImGui::PopStyleColor();
    
    ImGui::End();
}
