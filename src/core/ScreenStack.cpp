// Clean rewrite to remove stray lines
#include "ScreenStack.h"
#include "Settings.h"
#include "../screens/menus/MainMenu.h"
#include "../screens/menus/OptionsMenu.h"
#include "../screens/menus/PauseMenu.h"
#include "../screens/menus/MenuAction.h"
#include "../screens/GameScreen.h"
#include "../screens/FriendsScreen.h"
#include "../screens/LobbyScreen.h"
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

void ScreenStack::update() {
    if (screens.empty()) return;

    for (int i = static_cast<int>(screens.size()) - 1; i >= 0; --i) {
        screens[i]->update();
        if (!screens[i]->isOverlay()) break;
    }

    MenuAction action = screens.back()->getMenuAction();
    if (action == MenuAction::NONE) return;

    switch (action) {
        case MenuAction::START_SINGLEPLAYER:
            pop();
            push(std::make_unique<GameScreen>(true));
            break;
        case MenuAction::START_GAME:
            pop();
            push(std::make_unique<GameScreen>());
            break;
        case MenuAction::OPTIONS:
            push(std::make_unique<OptionsMenu>());
            break;
        case MenuAction::QUIT:
            clear();
            break;
        case MenuAction::MAIN_MENU:
            while (screens.size() > 1) pop();
            pop();
            push(std::make_unique<MainMenu>());
            break;
        case MenuAction::RESUME:
            pop();
            break;
        case MenuAction::PAUSE:
            push(std::make_unique<PauseMenu>());
            break;
        case MenuAction::FRIENDS:
            push(std::make_unique<FriendsScreen>());
            break;
        case MenuAction::LOBBIES:
            push(std::make_unique<LobbyScreen>());
            break;
        case MenuAction::SET_WINDOWED:
            Settings::setFullscreen(false);
            break;
        case MenuAction::SET_FULLSCREEN:
            Settings::setFullscreen(true);
            break;
        case MenuAction::TOGGLE_FULLSCREEN:
        case MenuAction::NONE:
        default:
            break;
    }

    screens.back()->resetMenuAction();
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
