#include "ImGuiManager.h"
#include "GraphicsContext.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>

ImGuiManager::~ImGuiManager() {
    if (initialized) {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
}

bool ImGuiManager::initialize(GraphicsContext* gfx) {
    if (!gfx) {
        std::cerr << "[ImGuiManager] GraphicsContext is null" << std::endl;
        return false;
    }

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    setDarkTheme();

    // Setup ImGui SDL2 backend
    if (!ImGui_ImplSDL2_InitForSDLRenderer(gfx->getWindow(), gfx->getRenderer())) {
        std::cerr << "[ImGuiManager] Failed to initialize ImGui SDL2 backend" << std::endl;
        ImGui::DestroyContext();
        return false;
    }

    // Setup ImGui SDL_Renderer backend
    if (!ImGui_ImplSDLRenderer2_Init(gfx->getRenderer())) {
        std::cerr << "[ImGuiManager] Failed to initialize ImGui SDLRenderer2 backend" << std::endl;
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    initialized = true;
    std::cout << "[ImGuiManager] ImGui initialized with SDL2 backend" << std::endl;
    return true;
}

void ImGuiManager::beginFrame() {
    if (!initialized) return;

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::endFrame(GraphicsContext* gfx) {
    if (!initialized || !gfx) return;

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::processEvent(const SDL_Event* event) {
    if (!initialized || !event) return;
    ImGui_ImplSDL2_ProcessEvent(event);
}

void ImGuiManager::setDarkTheme() {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Modern dark theme with better contrast and visual hierarchy
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowRounding = 8.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(12.0f, 8.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
    style.WindowPadding = ImVec2(16.0f, 16.0f);
    
    // Color palette: Modern blue accent with cool grays
    const ImVec4 accent_blue = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);     // Bright blue
    const ImVec4 accent_light = ImVec4(0.35f, 0.68f, 1.0f, 1.0f);      // Light blue
    const ImVec4 bg_dark = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);          // Very dark bg
    const ImVec4 bg_darker = ImVec4(0.04f, 0.04f, 0.05f, 1.0f);        // Almost black
    const ImVec4 surface = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);          // Dark surface
    const ImVec4 text_primary = ImVec4(0.95f, 0.95f, 0.97f, 1.0f);     // Primary text
    const ImVec4 text_secondary = ImVec4(0.70f, 0.70f, 0.72f, 1.0f);   // Secondary text
    
    ImVec4* colors = style.Colors;
    
    // Window and backgrounds
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.11f, 0.98f);
    colors[ImGuiCol_ChildBg] = bg_dark;
    colors[ImGuiCol_PopupBg] = surface;
    colors[ImGuiCol_FrameBg] = surface;
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.23f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.28f, 1.0f);
    
    // Buttons - Premium look with better hover state
    colors[ImGuiCol_Button] = accent_blue;
    colors[ImGuiCol_ButtonHovered] = accent_light;
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.48f, 0.88f, 1.0f);
    
    // Headers
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.27f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = accent_light;
    colors[ImGuiCol_HeaderActive] = accent_blue;
    
    // Text
    colors[ImGuiCol_Text] = text_primary;
    colors[ImGuiCol_TextDisabled] = text_secondary;
    
    // Separators and borders
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.28f, 1.0f);
    colors[ImGuiCol_SeparatorHovered] = accent_light;
    colors[ImGuiCol_SeparatorActive] = accent_blue;
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = bg_dark;
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.33f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.43f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.53f, 1.0f);
    
    // Checkboxes, radio, sliders
    colors[ImGuiCol_CheckMark] = accent_light;
    colors[ImGuiCol_SliderGrab] = accent_blue;
    colors[ImGuiCol_SliderGrabActive] = accent_light;
    
    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_TabHovered] = accent_light;
    colors[ImGuiCol_TabActive] = accent_blue;
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.13f, 1.0f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    
    // Titles
    colors[ImGuiCol_TitleBg] = bg_darker;
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = bg_darker;
}

void ImGuiManager::setLightTheme() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowRounding = 8.0f;
    style.FrameBorderSize = 0.0f;
    style.FramePadding = ImVec2(10.0f, 7.0f);
    style.ItemSpacing = ImVec2(12.0f, 8.0f);
    
    ImVec4* colors = style.Colors;
    const ImVec4 accent_blue = ImVec4(0.20f, 0.50f, 0.90f, 1.0f);
    const ImVec4 accent_light = ImVec4(0.40f, 0.65f, 0.95f, 1.0f);
    
    colors[ImGuiCol_Button] = accent_blue;
    colors[ImGuiCol_ButtonHovered] = accent_light;
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.45f, 0.85f, 1.0f);
    colors[ImGuiCol_CheckMark] = accent_blue;
    colors[ImGuiCol_SliderGrab] = accent_blue;
    colors[ImGuiCol_SliderGrabActive] = accent_light;
    colors[ImGuiCol_Header] = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = accent_light;
    colors[ImGuiCol_HeaderActive] = accent_blue;
}
