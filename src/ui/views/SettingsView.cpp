#include "SettingsView.h"
#include "core/Settings.h"
#include "core/KeyBindings.h"
#include "ui/components/UIComponents.h"
#include <imgui.h>
#include <algorithm>

SettingsView::SettingsView(bool isOverlay) : overlay(isOverlay) {}

void SettingsView::update() {
    // Countdown for applied confirmation message
    if (appliedTimer > 0.0f) {
        float dt = ImGui::GetIO().DeltaTime;
        appliedTimer = std::max(0.0f, appliedTimer - dt);
    }
}

void SettingsView::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(550, 0), ImGuiCond_FirstUseEver);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.16f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.68f, 0.98f, 0.3f));
    
    ImGui::Begin("##Options", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    
    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    ImVec2 title_size = ImGui::CalcTextSize("OPTIONS");
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_size.x * 1.8f) * 0.5f);
    ImGui::TextUnformatted("OPTIONS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // One-time initialization from current settings
    if (!initialized) {
        pendingDisplayMode = Settings::fullscreen ? 1 : 0;
        pendingLeftHanded = Settings::leftyMode;
        pendingAIDifficulty = Settings::aiDifficulty;
        initialized = true;
    }

    // Display Settings Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
    ImGui::Text("DISPLAY");
    ImGui::PopStyleColor();
    ImGui::Spacing();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::RadioButton("Windowed", &pendingDisplayMode, 0);
    ImGui::SameLine();
    ImGui::TextDisabled("(Recommended)");

    ImGui::RadioButton("Fullscreen", &pendingDisplayMode, 1);
    ImGui::PopStyleVar();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Audio Settings Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
    ImGui::Text("AUDIO");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    static float master_volume = 1.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::SliderFloat("Master Volume", &master_volume, 0.0f, 1.0f, "%.0f%%");
    ImGui::PopStyleVar();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // AI Settings Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
    ImGui::Text("AI DIFFICULTY");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    const char* difficulties[] = { "Very Easy", "Easy", "Medium", "Hard", "Veteran" };
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::Combo("Singleplayer AI", &pendingAIDifficulty, difficulties, 5);
    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::TextDisabled("Affects all AI opponents in singleplayer mode");
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Controls/Keybindings Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.80f, 1.0f));
    ImGui::Text("CONTROLS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Left-handed mode toggle
    ImGui::Checkbox("Left-Handed Mode", &pendingLeftHanded);
    ImGui::Spacing();
    
    // Display current keybindings
    ImGui::TextDisabled("Movement Keys:");
    ImGui::Indent();
    if (pendingLeftHanded) {
        ImGui::BulletText("Up: I");
        ImGui::BulletText("Down: K");
        ImGui::BulletText("Left: J");
        ImGui::BulletText("Right: L");
    } else {
        ImGui::BulletText("Up: W");
        ImGui::BulletText("Down: S");
        ImGui::BulletText("Left: A");
        ImGui::BulletText("Right: D");
    }
    ImGui::Unindent();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Buttons
    const float button_width = 200.0f;
    const float button_spacing = 15.0f;
    float center_x = (ImGui::GetWindowWidth() - (button_width * 2 + button_spacing)) * 0.5f;
    
    ImGui::SetCursorPosX(center_x);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    
    if (ImGui::Button("Apply", ImVec2(button_width, 40))) {
        bool changed = false;

        // Apply display mode via ScreenTransition
        bool wantFullscreen = (pendingDisplayMode == 1);
        if (wantFullscreen != Settings::fullscreen) {
            action = wantFullscreen ? ScreenTransition::SET_FULLSCREEN : ScreenTransition::SET_WINDOWED;
            changed = true;
        }

        // Apply handedness
        if (pendingLeftHanded != Settings::leftyMode) {
            Settings::toggleLeftyMode();
            changed = true;
        }

        // Apply AI difficulty
        if (pendingAIDifficulty != Settings::aiDifficulty) {
            Settings::setAIDifficulty(pendingAIDifficulty);
            changed = true;
        }

        if (changed) {
            appliedTimer = 2.0f; // show confirmation for 2 seconds
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(center_x + button_width + button_spacing);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button(overlay ? "Resume" : "Back", ImVec2(button_width, 40))) {
        action = overlay ? ScreenTransition::RESUME_PREVIOUS : ScreenTransition::TO_MAIN_MENU;
    }
    ImGui::PopStyleColor(3);

    if (appliedTimer > 0.0f) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.3f, 1.0f), "Settings applied");
    }
    
    ImGui::Spacing();
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}
