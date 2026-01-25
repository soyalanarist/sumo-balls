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
    
    // Customize dark theme
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowRounding = 4.0f;
    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
}

void ImGuiManager::setLightTheme() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.WindowRounding = 4.0f;
}
