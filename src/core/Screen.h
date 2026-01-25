#pragma once

// Forward declare to avoid circular includes
enum class MenuAction;

class Screen {
public:
    virtual ~Screen() = default;

    virtual void update() = 0;
    virtual void render() = 0;

    virtual bool isOverlay() const { return false; }
    
    // Menu action interface - screens that support menus override this
    virtual MenuAction getMenuAction() const;
    virtual void resetMenuAction() {}
};