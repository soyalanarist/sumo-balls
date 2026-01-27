#include "GraphicsContext.h"
#include "ImGuiManager.h"
#include <iostream>

GraphicsContext::~GraphicsContext() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

bool GraphicsContext::initialize(const std::string& title, int w, int h, bool fs) {
    width = w;
    height = h;
    fullscreen = fs;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "[GraphicsContext] Failed to initialize SDL2: " << SDL_GetError() << std::endl;
        return false;
    }
    
    Uint32 flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
    );
    
    if (!window) {
        std::cerr << "[GraphicsContext] Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }
    
    renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    
    if (!renderer) {
        std::cerr << "[GraphicsContext] Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    std::cout << "[GraphicsContext] Initialized SDL2 window (" << width << "x" << height << ")" << std::endl;
    
    return true;
}

void GraphicsContext::setWindowTitle(const std::string& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

void GraphicsContext::setFullscreen(bool fs) {
    if (fullscreen == fs) return;
    
    fullscreen = fs;
    if (window) {
        Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
        SDL_SetWindowFullscreen(window, flags);
    }
}

void GraphicsContext::toggleFullscreen() {
    setFullscreen(!fullscreen);
}

void GraphicsContext::beginFrame() {
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);
}

void GraphicsContext::endFrame() {
    SDL_RenderPresent(renderer);
}

bool GraphicsContext::processEvents(ImGuiManager* imgui) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Pass event to ImGui first
        if (imgui) {
            imgui->processEvent(&event);
        }
        
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                return true;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    running = false;
                    return true;
                }
                break;
        }
        
        // ESC handling moved to screens (they can override in handleInput)
        // Only quit on ESC if no screen handles it
    }
    return false;
}
