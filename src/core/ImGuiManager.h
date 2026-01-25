#pragma once

#include <imgui.h>
#include <memory>

// Forward declarations
union SDL_Event;


class GraphicsContext;

class ImGuiManager {
public:
    ImGuiManager() = default;
    ~ImGuiManager();

    // Initialize ImGui with SDL2 backend
    bool initialize(GraphicsContext* gfx);

    // Frame management
    void beginFrame();
    void endFrame(GraphicsContext* gfx);
    
    // Event handling - pass SDL events to ImGui
    void processEvent(const SDL_Event* event);

    // Style/theming
    void setDarkTheme();
    void setLightTheme();

    // Getters
    ImGuiIO* getIO() { return &ImGui::GetIO(); }

private:
    bool initialized = false;
};
