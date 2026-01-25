#pragma once

#include <imgui.h>
#include <string>

namespace UIComponents {

// Color constants for consistent theming
namespace Colors {
    constexpr ImVec4 ACCENT_BLUE = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    constexpr ImVec4 ACCENT_LIGHT = ImVec4(0.35f, 0.68f, 1.0f, 1.0f);
    constexpr ImVec4 SUCCESS = ImVec4(0.40f, 0.90f, 0.40f, 1.0f);
    constexpr ImVec4 ERROR = ImVec4(1.00f, 0.40f, 0.40f, 1.0f);
    constexpr ImVec4 WARNING = ImVec4(1.00f, 0.80f, 0.40f, 1.0f);
    constexpr ImVec4 TEXT_PRIMARY = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);
    constexpr ImVec4 TEXT_SECONDARY = ImVec4(0.70f, 0.70f, 0.72f, 1.0f);
    constexpr ImVec4 BG_DARK = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);
    constexpr ImVec4 SURFACE = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
}

// Common button with consistent styling
bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(-1, 48));
bool SecondaryButton(const char* label, const ImVec2& size = ImVec2(-1, 40));
bool DangerButton(const char* label, const ImVec2& size = ImVec2(-1, 40));

// Centered heading text
void CenteredHeading(const char* text, const ImVec4& color = Colors::ACCENT_BLUE, float scale = 1.5f);

// Centered subheading
void CenteredSubheading(const char* text, const ImVec4& color = Colors::TEXT_SECONDARY);

// Status message with icon
void SuccessMessage(const char* message);
void ErrorMessage(const char* message);
void WarningMessage(const char* message);
void InfoMessage(const char* message);

// Decorative elements
void HorizontalSeparator(float thickness = 2.0f, const ImVec4& color = Colors::ACCENT_BLUE);
void VerticalSeparator(float height = 0.0f);

// Loading spinner animation
void LoadingSpinner(float size = 20.0f);

// Section headers with underline
void SectionHeader(const char* label);

} // namespace UIComponents
