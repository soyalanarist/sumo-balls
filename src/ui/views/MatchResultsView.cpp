#include "MatchResultsView.h"
#include <imgui.h>

MatchResultsView::MatchResultsView() = default;

void MatchResultsView::update() {
    // No background logic yet
}

void MatchResultsView::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Always);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.11f, 0.95f));
    
    ImGui::Begin("Game Over", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Accent border
    ImU32 border_color = ImGui::GetColorU32(ImVec4(0.35f, 0.68f, 0.98f, 0.3f));
    draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), 
        border_color, 12.0f, 0, 2.0f);
    
    ImGui::Spacing();
    
    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.3f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    float title_width = ImGui::CalcTextSize("MATCH OVER").x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_width * 1.8f) * 0.5f);
    ImGui::TextUnformatted("MATCH OVER");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Results section (placeholder)
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
    ImGui::BeginChild("##ResultsBox", ImVec2(0, 140), true);
    
    ImGui::TextUnformatted("MATCH RESULTS:");
    ImGui::Spacing();
    ImGui::Text("Position: 1st Place");
    ImGui::Text("Kills: 5");
    ImGui::Text("Experience Gained: +150");
    ImGui::Text("Credits Earned: +500");
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Buttons
    const float button_width = 200.0f;
    const float button_spacing = 15.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    if (ImGui::Button("Play Again *", ImVec2(button_width, 40))) {
        // Play again
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::PopStyleColor(3);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button("Main Menu", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_MAIN_MENU;
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}
