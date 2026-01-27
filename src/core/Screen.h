#pragma once

// Forward declare to avoid circular includes
enum class ScreenTransition;
union SDL_Event;

class Screen {
public:
    virtual ~Screen() = default;

    virtual void update() = 0;
    virtual void render() = 0;

    virtual bool isOverlay() const { return false; }
    
    // Input handling with event bubbling support
    // Return true if event was consumed and should not bubble to screens below
    // Return false to allow the event to propagate (for overlays and multi-layer handling)
    virtual bool handleInput(const SDL_Event& event) { (void)event; return false; }
    
    // Optional: Allow screens to indicate whether they want to receive input events
    // Used by ScreenStack for optimization and to skip non-interactive screens
    virtual bool wantsInput() const { return true; }
    
    // Screen transition interface - screens override to communicate navigation actions
    virtual ScreenTransition getTransition() const;
    virtual void resetTransition() {}
};
