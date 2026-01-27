#include "FriendsView.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>

FriendsView::FriendsView() {
    refreshData();
}

FriendsView::~FriendsView() = default;

void FriendsView::update() {
    // Auto-refresh friends and requests every 30 seconds
    uint32_t now = SDL_GetTicks();
    if (now - lastRefreshTime > refreshInterval) {
        refreshData();
        lastRefreshTime = now;
    }
}

void FriendsView::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    float targetWidth = (vp ? vp->Size.x : ImGui::GetIO().DisplaySize.x) * 0.6f;
    if (targetWidth < 500.0f) targetWidth = 500.0f;
    if (targetWidth > 900.0f) targetWidth = 900.0f;
    
    float targetHeight = (vp ? vp->Size.y : ImGui::GetIO().DisplaySize.y) * 0.75f;
    ImGui::SetNextWindowSize(ImVec2(targetWidth, targetHeight), ImGuiCond_Always);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.16f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.68f, 0.98f, 0.3f));
    
    ImGui::Begin("Friends", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
    
    // Current user display
    const User& currentUser = SocialManager::instance().getCurrentUser();
    if (currentUser.id != 0) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
        ImGui::Text("Logged in as:");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        ImGui::Text("%s", currentUser.getDisplayName().c_str());
        ImGui::PopStyleColor();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Title with Add Friend button
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.6f);
    ImVec2 title_size = ImGui::CalcTextSize("FRIENDS");
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_size.x * 1.6f - 150) * 0.5f);
    ImGui::TextUnformatted("FRIENDS");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::SameLine(ImGui::GetWindowWidth() - 145);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    if (ImGui::Button("+ Add Friend", ImVec2(130, 32))) {
        showAddFriend = true;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Status/Error messages
    if (!statusMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", statusMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    
    if (!errorMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", errorMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    
    // Main content in child window for scrolling
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.13f, 1.0f));
    ImGui::BeginChild("FriendsContent", ImVec2(0, -50), true);
    ImGui::PopStyleColor();
    
    renderPendingRequests();
    renderFriendsList();
    
    ImGui::EndChild();
    
    // Footer buttons
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    const float button_width = 140.0f;
    const float button_spacing = 10.0f;
    float button_x = (ImGui::GetWindowWidth() - (button_width * 2 + button_spacing)) * 0.5f;
    
    ImGui::SetCursorPosX(button_x);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button("Refresh", ImVec2(button_width, 36))) {
        refreshData();
    }
    
    ImGui::SameLine(0, button_spacing);
    if (ImGui::Button("Main Menu", ImVec2(button_width, 36))) {
        action = ScreenTransition::TO_MAIN_MENU;
    }
    
    ImGui::PopStyleColor(3);
    
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    
    // Add friend dialog
    if (showAddFriend) {
        renderAddFriendDialog();
    }
}

void FriendsView::renderPendingRequests() {
    const auto& requests = SocialManager::instance().getPendingRequests();
    
    if (requests.empty()) {
        return;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.78f, 0.36f, 1.0f));
    ImGui::Text("PENDING REQUESTS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    for (const auto& req : requests) {
        ImGui::PushID(req.id);
        
        // Request item with padding
        ImGui::Indent(8.0f);
        ImGui::TextWrapped("%s", req.getDisplayName().c_str());
        ImGui::Unindent(8.0f);
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 210);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.36f, 0.78f, 0.36f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.87f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.68f, 0.28f, 1.0f));
        std::string acceptBtn = std::string("Accept##") + std::to_string(req.id);
        if (ImGui::Button(acceptBtn.c_str(), ImVec2(85, 28))) {
            onAcceptRequest(req.from_user_id);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine(0, 8);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.78f, 0.36f, 0.36f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.87f, 0.45f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.68f, 0.28f, 0.28f, 1.0f));
        std::string declineBtn = std::string("Decline##") + std::to_string(req.id);
        if (ImGui::Button(declineBtn.c_str(), ImVec2(85, 28))) {
            onDeclineRequest(req.from_user_id);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Spacing();
        ImGui::PopID();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

void FriendsView::renderFriendsList() {
    const auto& friends = SocialManager::instance().getFriends();
    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.36f, 0.78f, 0.36f, 1.0f));
    ImGui::Text("FRIENDS");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (friends.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 0.8f));
        ImGui::TextWrapped("You don't have any friends yet. Click 'Add Friend' to send a friend request!");
        ImGui::PopStyleColor();
        return;
    }
    
    for (const auto& friend_ : friends) {
        ImGui::PushID(friend_.id);
        
        // Friend item with proper padding
        ImGui::Indent(8.0f);
        ImGui::Text("%s", friend_.getDisplayName().c_str());
        ImGui::Unindent(8.0f);
        
        // Action buttons
        ImGui::SameLine(ImGui::GetWindowWidth() - 210);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
        std::string inviteBtn = std::string("Invite##") + std::to_string(friend_.id);
        if (ImGui::Button(inviteBtn.c_str(), ImVec2(85, 28))) {
            statusMessage = "Lobby invites coming soon!";
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine(0, 8);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.78f, 0.36f, 0.36f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.87f, 0.45f, 0.45f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.68f, 0.28f, 0.28f, 1.0f));
        std::string removeBtn = std::string("Remove##") + std::to_string(friend_.id);
        if (ImGui::Button(removeBtn.c_str(), ImVec2(85, 28))) {
            onRemoveFriend(friend_.friend_id);
        }
        ImGui::PopStyleColor(3);
        
        ImGui::Spacing();
        ImGui::PopID();
    }
}

void FriendsView::renderAddFriendDialog() {
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 0), ImGuiCond_Always);
    
    if (ImGui::Begin("Add Friend", &showAddFriend, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("Enter your friend's handle or username:");
        ImGui::Spacing();
        
        ImGui::SetNextItemWidth(-1);
        bool enterPressed = ImGui::InputText("##friendsearch", friendSearchInput, sizeof(friendSearchInput),
                                             ImGuiInputTextFlags_EnterReturnsTrue);
        
        ImGui::Spacing();
        
        ImGui::BeginDisabled(isSearching || strlen(friendSearchInput) < 3);
        if ((ImGui::Button("Send Request", ImVec2(-1, 0)) || enterPressed) && !isSearching) {
            onSendRequest();
        }
        ImGui::EndDisabled();
        
        if (isSearching) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 1.0f));
            float time = ImGui::GetTime();
            const char* spinner[] = {"|", "/", "-", "\\"};
            ImGui::Text("%s Sending request...", spinner[(int)(time * 4) % 4]);
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}

void FriendsView::onAcceptRequest(int64_t fromUserId) {
    statusMessage.clear();
    errorMessage.clear();
    
    SocialManager::instance().acceptFriendRequest(fromUserId,
        [this](bool success, const std::string& message) {
            if (success) {
                statusMessage = "Friend request accepted!";
                errorMessage.clear();
            } else {
                errorMessage = "Failed to accept request: " + message;
                statusMessage.clear();
            }
        });
}

void FriendsView::onDeclineRequest(int64_t fromUserId) {
    statusMessage.clear();
    errorMessage.clear();
    
    SocialManager::instance().declineFriendRequest(fromUserId,
        [this](bool success, const std::string& message) {
            if (success) {
                statusMessage = "Friend request declined";
                errorMessage.clear();
            } else {
                errorMessage = "Failed to decline request: " + message;
                statusMessage.clear();
            }
        });
}

void FriendsView::onRemoveFriend(int64_t friendId) {
    statusMessage.clear();
    errorMessage.clear();
    
    SocialManager::instance().removeFriend(friendId,
        [this](bool success, const std::string& message) {
            if (success) {
                statusMessage = "Friend removed";
                errorMessage.clear();
            } else {
                errorMessage = "Failed to remove friend: " + message;
                statusMessage.clear();
            }
        });
}

void FriendsView::onSendRequest() {
    std::string handle = friendSearchInput;
    if (handle.empty()) return;
    
    isSearching = true;
    statusMessage.clear();
    errorMessage.clear();
    
    SocialManager::instance().sendFriendRequest(handle,
        [this](bool success, const std::string& message) {
            isSearching = false;
            if (success) {
                statusMessage = "Friend request sent!";
                errorMessage.clear();
                showAddFriend = false;
                friendSearchInput[0] = '\0';
            } else {
                errorMessage = message;
                statusMessage.clear();
            }
        });
}

void FriendsView::refreshData() {
    SocialManager::instance().refreshAll();
    lastRefreshTime = SDL_GetTicks();
}

ScreenTransition FriendsView::getTransition() const {
    return action;
}

void FriendsView::resetTransition() {
    action = ScreenTransition::NONE;
}

void FriendsView::removeFriend(const std::string & /*username*/) {
    // Legacy method - now handled by onRemoveFriend
}

void FriendsView::setStatusMessage(const std::string & message) {
    statusMessage = message;
}
