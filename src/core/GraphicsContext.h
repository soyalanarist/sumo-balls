#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <string>

class ImGuiManager;  // Forward declaration

class GraphicsContext {
public:
    GraphicsContext() = default;
    ~GraphicsContext();

    // Initialize SDL2 window and renderer
    bool initialize(const std::string& title, int width, int height, bool fullscreen = false);
    
    // Window management
    void setWindowTitle(const std::string& title);
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    
    // Frame management
    void beginFrame();
    void endFrame();
    
    // Queries
    SDL_Window* getWindow() { return window; }
    SDL_Renderer* getRenderer() { return renderer; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isRunning() const { return running; }
    void quit() { running = false; }
    
    // Event handling (returns true if window should close)
    bool processEvents(ImGuiManager* imgui = nullptr);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    int width = 1200;
    int height = 900;
    bool running = true;
    bool fullscreen = false;
};
