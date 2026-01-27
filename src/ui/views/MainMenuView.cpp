#include "MainMenuView.h"
#include <imgui.h>
#include <cmath>

MainMenuView::MainMenuView() {}

void MainMenuView::update() {
    // ImGui handles all UI updates automatically
}

void MainMenuView::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##MainMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float window_width = ImGui::GetWindowWidth();
    float window_height = ImGui::GetWindowHeight();
    
    // Gradient background
    ImU32 grad_top = ImGui::GetColorU32(ImVec4(0.10f, 0.12f, 0.16f, 1.0f));
    ImU32 grad_bot = ImGui::GetColorU32(ImVec4(0.06f, 0.07f, 0.09f, 1.0f));
    draw_list->AddRectFilledMultiColor(
        ImVec2(0, 0),
        ImVec2(window_width, window_height),
        grad_top, grad_top, grad_bot, grad_bot
    );
    
    // Decorative accent elements
    ImU32 accent = ImGui::GetColorU32(ImVec4(0.35f, 0.68f, 0.98f, 0.1f));
    draw_list->AddCircle(ImVec2(window_width * 0.2f, -200), 400, accent, 0, 3.0f);
    draw_list->AddCircle(ImVec2(window_width * 0.8f, window_height + 200), 350, accent, 0, 3.0f);
    
    // Title section
    ImGui::SetCursorPosY(40);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(2.2f);
    float title_width = ImGui::CalcTextSize("SUMO BALLS").x;
    ImGui::SetCursorPosX((window_width - title_width * 2.2f) * 0.5f);
    ImGui::TextUnformatted("SUMO BALLS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    // Subtitle
    ImGui::SetCursorPosY(120);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 0.9f));
    float subtitle_width = ImGui::CalcTextSize("COMPETITIVE PHYSICS BATTLE ARENA").x;
    ImGui::SetCursorPosX((window_width - subtitle_width) * 0.5f);
    ImGui::TextUnformatted("COMPETITIVE PHYSICS BATTLE ARENA");
    ImGui::PopStyleColor();
    
    // Menu content
    ImGui::SetCursorPosY(200);
    const float button_width = 300.0f;
    const float button_height = 50.0f;
    const float button_spacing = 15.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 15.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    // Play button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    if (ImGui::Button("PLAY *", ImVec2(button_width, button_height))) {
        action = ScreenTransition::TO_LOBBY;
    }
    
    // Quick Match button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::BeginDisabled(true);
    if (ImGui::Button("QUICK MATCH *", ImVec2(button_width, button_height))) {}
    ImGui::EndDisabled();
    ImGui::PopStyleColor(4);
    
    // Secondary buttons
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    // Friends button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing * 2);
    if (ImGui::Button("FRIENDS", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_FRIENDS;
    }
    
    // Battle Pass button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::BeginDisabled(true);
    if (ImGui::Button("BATTLE PASS *", ImVec2(button_width, 40))) {}
    ImGui::EndDisabled();
    
    // Stats button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::BeginDisabled(true);
    if (ImGui::Button("STATS *", ImVec2(button_width, 40))) {}
    ImGui::EndDisabled();
    
    // Settings button
    ImGui::SetCursorPosX((window_width - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (ImGui::Button("SETTINGS", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_OPTIONS;
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    
    // Footer
    ImGui::SetCursorPosY(window_height - 60.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.52f, 0.58f, 0.5f));
    const char* footer = "v0.1.0 | Competitive Physics Arena";
    float footer_width = ImGui::CalcTextSize(footer).x;
    ImGui::SetCursorPosX((window_width - footer_width) * 0.5f);
    ImGui::TextUnformatted(footer);
    ImGui::PopStyleColor();
    
    ImGui::End();
}
