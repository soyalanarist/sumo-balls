#pragma once

#include "core/Screen.h"
#include "core/ScreenTransition.h"
#include "network/SocialManager.h"
#include <string>
#include <cstdint>

class FriendsView : public Screen {
public:
    FriendsView();
    ~FriendsView() override;

    void update() override;
    void render() override;
    bool isOverlay() const override { return false; }

    ScreenTransition getTransition() const override;
    void resetTransition() override;

    void removeFriend(const std::string &username);
    void setStatusMessage(const std::string &message);

private:
    ScreenTransition action = ScreenTransition::NONE;
    std::string statusMessage;
    std::string errorMessage;
    
    // Add friend UI state
    bool showAddFriend = false;
    char friendSearchInput[51] = "";
    bool isSearching = false;
    
    // Auto-refresh timer
    uint32_t lastRefreshTime = 0;
    const uint32_t refreshInterval = 30000; // 30 seconds
    
    // UI sections
    void renderPendingRequests();
    void renderFriendsList();
    void renderAddFriendDialog();
    
    // Actions
    void onAcceptRequest(int64_t fromUserId);
    void onDeclineRequest(int64_t fromUserId);
    void onRemoveFriend(int64_t friendId);
    void onSendRequest();
    void refreshData();
};
