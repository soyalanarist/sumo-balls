#include "OptionsMenu.h"
#include "../../core/Settings.h"
#include "../../ui/UIComponents.h"
#include <imgui.h>

OptionsMenu::OptionsMenu(bool isOverlay) : overlay(isOverlay) {}

void OptionsMenu::update() {
    // No background logic yet
}

void OptionsMenu::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 0), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("##Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Decorative top bar
    ImU32 accent_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + 4), accent_color);
    
    ImGui::Spacing();
    
    // Title
    UIComponents::CenteredHeading("⚙️  Options");
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Display Settings Section
    UIComponents::SectionHeader("Display");
    ImGui::Spacing();
    
    static int display_mode = 0;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    if (ImGui::RadioButton("Windowed", &display_mode, 0)) {
        action = MenuAction::SET_WINDOWED;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(Recommended)");
    
    if (ImGui::RadioButton("Fullscreen", &display_mode, 1)) {
        action = MenuAction::SET_FULLSCREEN;
    }
    ImGui::PopStyleVar();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Audio Settings Section
    UIComponents::SectionHeader("Audio");
    ImGui::Spacing();
    
    static float master_volume = 1.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::SliderFloat("Master Volume", &master_volume, 0.0f, 1.0f, "%.0f%%");
    ImGui::PopStyleVar();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Gameplay Settings Section
    UIComponents::SectionHeader("Gameplay");
    ImGui::Spacing();
    
    static bool show_fps = true;
    static bool screen_shake = true;
    
    ImGui::Checkbox("Show FPS Counter", &show_fps);
    ImGui::Checkbox("Screen Shake Effects", &screen_shake);
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Footer
    UIComponents::HorizontalSeparator(1.5f);
    ImGui::Spacing();
    
    // Buttons
    const float button_width = 200.0f;
    const float button_spacing = 20.0f;
    float center_x = (ImGui::GetWindowWidth() - (button_width * 2 + button_spacing)) * 0.5f;
    
    ImGui::SetCursorPosX(center_x);
    if (UIComponents::PrimaryButton("Apply", ImVec2(button_width, 40))) {
        // Save settings
    }
    ImGui::SameLine();
    ImGui::SetCursorPosX(center_x + button_width + button_spacing);
    if (UIComponents::SecondaryButton(overlay ? "Resume" : "Back", ImVec2(button_width, 40))) {
        action = overlay ? MenuAction::RESUME : MenuAction::MAIN_MENU;
    }
    
    ImGui::End();
}
