#include "PauseMenuView.h"
#include "ui/components/UIComponents.h"
#include <imgui.h>

PauseMenuView::PauseMenuView() = default;

void PauseMenuView::update(){
    // No background logic yet
}

void PauseMenuView::render() {
    // Semi-transparent overlay
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##PauseOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 overlay_color = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
    draw_list->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, overlay_color);
    
    ImGui::End();
    
    // Pause menu centered window
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_FirstUseEver);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.16f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.68f, 0.98f, 0.3f));
    
    ImGui::Begin("##PauseMenu", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    
    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    ImVec2 title_size = ImGui::CalcTextSize("PAUSED");
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_size.x * 1.8f) * 0.5f);
    ImGui::TextUnformatted("PAUSED");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Buttons
    const float button_width = 280.0f;
    const float button_height = 45.0f;
    const float button_spacing = 12.0f;
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    
    if (ImGui::Button("Resume Game", ImVec2(button_width, button_height))) {
        action = ScreenTransition::RESUME_PREVIOUS;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button("Options", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_OPTIONS;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button("Main Menu", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_MAIN_MENU;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing * 1.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.80f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.30f, 0.30f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.15f, 0.15f, 1.0f));
    
    if (ImGui::Button("Quit Game", ImVec2(button_width, 40))) {
        action = ScreenTransition::QUIT;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::Spacing();
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}
