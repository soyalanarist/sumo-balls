#include "ScreenStack.h"
#include "Settings.h"
#include "GraphicsContext.h"
#include "ScreenTransition.h"
#include "ui/views/MainMenuView.h"
#include "ui/views/SettingsView.h"
#include "ui/views/PauseMenuView.h"
#include "ui/views/LoginView.h"
#include "ui/views/CreatePlayerView.h"
#include "ui/views/FriendsView.h"
#include "ui/views/LobbyView.h"
#include "ui/views/MatchResultsView.h"
#include "ui/scenes/MatchScene.h"
#include "ui/scenes/MatchResultsScene.h"
#include <iostream>

ScreenStack::ScreenStack(void*, void*) {}

void ScreenStack::push(std::unique_ptr<Screen> screen) {
    screens.push_back(std::move(screen));
}

void ScreenStack::pop() {
    if (!screens.empty()) {
        screens.pop_back();
    }
}

void ScreenStack::clear() {
    while (!screens.empty()) {
        screens.pop_back();
    }
}

bool ScreenStack::handleInput(const SDL_Event& event) {
    if (screens.empty()) return false;
    
    // Event bubbling: Process input top-to-bottom through overlays
    // - Top screen (non-overlay) gets first chance to consume event
    // - If top screen doesn't consume, check screens below it (overlays only)
    // - Stop at first non-overlay that doesn't consume the event
    // - If no screen consumes, return false to let global handlers process it
    
    for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
        // Skip screens that don't want input
        if (!screens[i]->wantsInput()) {
            continue;
        }
        
        // Let this screen handle the input
        if (screens[i]->handleInput(event)) {
            return true;  // Event consumed, stop bubbling
        }
        
        // If this is a non-overlay, stop the bubble chain
        // (don't let screens behind the top non-overlay get input)
        if (!screens[i]->isOverlay()) {
            break;
        }
    }
    
    return false;  // No screen consumed the event
}

void ScreenStack::update() {
    if (screens.empty()) return;

    for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
        screens[i]->update();
        if (!screens[i]->isOverlay()) break;
    }

    auto findGameScene = [&]() -> MatchScene* {
        for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
            auto* gs = dynamic_cast<MatchScene*>(screens[i].get());
            if (gs) return gs;
        }
        return nullptr;
    };

    // Process actions safely even if stack mutates
    while (!screens.empty()) {
        ScreenTransition action = screens.back()->getTransition();
        if (action == ScreenTransition::NONE) break;
        screens.back()->resetTransition();

        switch (action) {
            case ScreenTransition::TO_MATCH_SINGLEPLAYER:
                pop();
                push(std::make_unique<MatchScene>(true));
                break;
            case ScreenTransition::TO_MATCH:
                pop();
                push(std::make_unique<MatchScene>());
                break;
            case ScreenTransition::TO_OPTIONS:
                push(std::make_unique<SettingsView>());
                break;
            case ScreenTransition::QUIT:
                clear();
                if (graphics) graphics->quit();
                break;
            case ScreenTransition::TO_MAIN_MENU: {
                MatchScene* gs = findGameScene();
                if (gs) gs->setPaused(false);
                while (screens.size() > 1) pop();
                pop();
                push(std::make_unique<MainMenuView>());
                break;
            }
            case ScreenTransition::RESUME_PREVIOUS: {
                pop();
                MatchScene* gs = findGameScene();
                if (gs) gs->setPaused(false);
                break;
            }
            case ScreenTransition::PAUSE: {
                MatchScene* gs = findGameScene();
                if (gs) gs->setPaused(true);
                push(std::make_unique<PauseMenuView>());
                break;
            }
            case ScreenTransition::TO_FRIENDS:
                push(std::make_unique<FriendsView>());
                break;
            case ScreenTransition::TO_LOBBY:
                push(std::make_unique<LobbyView>());
                break;
            case ScreenTransition::TO_HANDLE_SETUP:
                pop();
                push(std::make_unique<CreatePlayerView>());
                break;
            case ScreenTransition::TO_LOGIN:
                push(std::make_unique<LoginView>());
                break;
            case ScreenTransition::TO_MATCH_RESULTS:
                push(std::make_unique<MatchResultsView>());
                break;
            case ScreenTransition::SET_WINDOWED:
                Settings::setFullscreen(false);
                if (graphics) graphics->setFullscreen(false);
                break;
            case ScreenTransition::SET_FULLSCREEN:
                Settings::setFullscreen(true);
                if (graphics) graphics->setFullscreen(true);
                break;
            case ScreenTransition::TOGGLE_FULLSCREEN:
            case ScreenTransition::NONE:
            default:
                break;
        }
    }
}

void ScreenStack::render() {
    for (int i = 0; i < static_cast<int>(screens.size()); ++i) {
        bool hasNonOverlayAbove = false;
        for (int j = i + 1; j < static_cast<int>(screens.size()); ++j) {
            if (!screens[j]->isOverlay()) {
                hasNonOverlayAbove = true;
                break;
            }
        }
        if (!hasNonOverlayAbove) {
            screens[i]->render();
        }
    }
}
