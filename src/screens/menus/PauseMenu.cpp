#include "PauseMenu.h"
#include "../../ui/UIComponents.h"
#include <imgui.h>

PauseMenu::PauseMenu() = default;

void PauseMenu::update(){
    // No background logic yet
}

void PauseMenu::render() {
    // Semi-transparent overlay
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##PauseOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 overlay_color = ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.6f));
    draw_list->AddRectFilled(ImVec2(0, 0), ImGui::GetIO().DisplaySize, overlay_color);
    
    ImGui::End();
    
    // Pause menu
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(450, 0), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("##PauseMenu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImDrawList* menu_draw_list = ImGui::GetWindowDrawList();
    ImVec2 menu_pos = ImGui::GetWindowPos();
    ImVec2 menu_size = ImGui::GetWindowSize();
    
    // Decorative top bar
    ImU32 accent_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    menu_draw_list->AddRectFilled(menu_pos, ImVec2(menu_pos.x + menu_size.x, menu_pos.y + 4), accent_color);
    
    ImGui::Spacing();
    
    // Title
    UIComponents::CenteredHeading("⏸️  PAUSED");
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Buttons with better styling
    const float button_width = 280.0f;
    const float button_height = 50.0f;
    const float button_spacing = 15.0f;
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    if (UIComponents::PrimaryButton("▶ Resume Game", ImVec2(button_width, button_height))) {
        action = MenuAction::RESUME;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (UIComponents::SecondaryButton("⚙️  Options", ImVec2(button_width, 40))) {
        action = MenuAction::OPTIONS;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
    if (UIComponents::SecondaryButton("🏠  Main Menu", ImVec2(button_width, 40))) {
        action = MenuAction::MAIN_MENU;
    }
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing * 1.5f);
    if (UIComponents::DangerButton("❌  Quit Game", ImVec2(button_width, 40))) {
        action = MenuAction::QUIT;
    }
    
    ImGui::End();
}