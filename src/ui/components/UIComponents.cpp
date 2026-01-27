#include "UIComponents.h"
#include <imgui.h>
#include <cmath>

namespace UIComponents {

bool PrimaryButton(const char* label, const ImVec2& size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 14.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::ACCENT_BLUE);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Colors::ACCENT_LIGHT);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.48f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TEXT_PRIMARY);
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    return result;
}

bool SecondaryButton(const char* label, const ImVec2& size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.27f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.37f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.40f, 0.40f, 0.42f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TEXT_PRIMARY);
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    return result;
}

bool DangerButton(const char* label, const ImVec2& size) {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.45f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.10f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TEXT_PRIMARY);
    
    bool result = ImGui::Button(label, size);
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    return result;
}

void CenteredHeading(const char* text, const ImVec4& color, float scale) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    float text_width = ImGui::CalcTextSize(text).x * scale;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_width) * 0.5f);
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Assuming fonts are set up
    ImGui::TextUnformatted(text);
    ImGui::PopFont();
    
    ImGui::PopStyleColor();
}

void CenteredSubheading(const char* text, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    float text_width = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_width) * 0.5f);
    ImGui::TextUnformatted(text);
    ImGui::PopStyleColor();
}

void SuccessMessage(const char* message) {
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::SUCCESS);
    ImGui::TextWrapped("Success: %s", message);
    ImGui::PopStyleColor();
}

void ErrorMessage(const char* message) {
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::ERROR);
    ImGui::TextWrapped("Error: %s", message);
    ImGui::PopStyleColor();
}

void WarningMessage(const char* message) {
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WARNING);
    ImGui::TextWrapped("Warning: %s", message);
    ImGui::PopStyleColor();
}

void InfoMessage(const char* message) {
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TEXT_SECONDARY);
    ImGui::TextWrapped("Info: %s", message);
    ImGui::PopStyleColor();
}

void HorizontalSeparator(float thickness, const ImVec4& color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    float width = ImGui::GetContentRegionAvail().x;
    
    ImU32 col32 = ImGui::GetColorU32(color);
    draw_list->AddLine(cursor_pos, ImVec2(cursor_pos.x + width, cursor_pos.y), col32, thickness);
    
    ImGui::Dummy(ImVec2(0, thickness + 8.0f));
}

void VerticalSeparator(float height) {
    ImGui::SameLine();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    
    if (height == 0.0f) {
        height = ImGui::GetTextLineHeight() * 2.0f;
    }
    
    ImU32 col32 = ImGui::GetColorU32(Colors::ACCENT_BLUE);
    draw_list->AddLine(cursor_pos, ImVec2(cursor_pos.x, cursor_pos.y + height), col32, 2.0f);
}

void LoadingSpinner(float size) {
    float time = ImGui::GetTime();
    float radius = size * 0.5f;
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 center = ImGui::GetCursorScreenPos();
    center.x += radius;
    center.y += radius;
    
    const int segments = 8;
    float angle_offset = time * 6.0f;
    
    for (int i = 0; i < segments; i++) {
        float angle1 = (2.0f * 3.14159f * i) / segments + angle_offset;
        float angle2 = (2.0f * 3.14159f * (i + 1)) / segments + angle_offset;
        
        float alpha = (float)(i + 1) / segments;
        ImVec4 color = Colors::ACCENT_BLUE;
        color.w = alpha;
        
        ImVec2 p1(center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius);
        ImVec2 p2(center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius);
        
        draw_list->AddLine(p1, p2, ImGui::GetColorU32(color), 2.0f);
    }
    
    ImGui::Dummy(ImVec2(size, size));
}

void SectionHeader(const char* label) {
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::ACCENT_BLUE);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    HorizontalSeparator(1.5f, Colors::ACCENT_BLUE);
}

} // namespace UIComponents
