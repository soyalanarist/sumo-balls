// Friends screen stub to satisfy build during migration

#include "FriendsScreen.h"
#include <imgui.h>

FriendsScreen::FriendsScreen() = default;
FriendsScreen::~FriendsScreen() = default;

void FriendsScreen::update() {
    // TODO: Implement friends list UI in ImGui
}

void FriendsScreen::render() {
    ImGui::Begin("Friends", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Friends (stub)");
    ImGui::Separator();
    ImGui::TextWrapped("This screen will show your friends list and requests.");
    if (ImGui::Button("Back to Main Menu")) {
        action = MenuAction::MAIN_MENU;
    }
    ImGui::End();
}

MenuAction FriendsScreen::getMenuAction() const {
    return action;
}

void FriendsScreen::resetMenuAction() {
    action = MenuAction::NONE;
}

void FriendsScreen::removeFriend(const std::string & /*username*/) {
    // TODO: Hook backend removal once implemented
}

void FriendsScreen::setStatusMessage(const std::string & message) {
    statusMessage = message;
}
