#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#include <optional>
// Data structures matching backend API
struct User {
    int64_t id = 0;
    std::string username;
    std::string handle;
    std::string tagline;  // 4-char alphanumeric identifier
    std::string email;
    bool needs_handle_setup = false;
    std::string created_at;
    
    std::string getDisplayName() const {
        if (!handle.empty() && !tagline.empty()) {
            return handle + "#" + tagline;
        }
        return handle.empty() ? username : handle;
    }
};

struct Friend {
    int64_t id = 0;
    int64_t user_id = 0;
    int64_t friend_id = 0;
    std::string status;  // "accepted"
    std::string friend_username;
    std::string friend_handle;
    std::string friend_tagline;
    
    std::string getDisplayName() const {
        if (!friend_handle.empty() && !friend_tagline.empty()) {
            return friend_handle + "#" + friend_tagline;
        }
        return friend_handle.empty() ? friend_username : friend_handle;
    }
};

struct FriendRequest {
    int64_t id = 0;
    int64_t from_user_id = 0;
    std::string from_username;
    std::string from_handle;
    std::string from_tagline;
    std::string created_at;
    
    std::string getDisplayName() const {
        if (!from_handle.empty() && !from_tagline.empty()) {
            return from_handle + "#" + from_tagline;
        }
        return from_handle.empty() ? from_username : from_handle;
    }
};

struct LobbyMember {
    int64_t user_id = 0;
    std::string username;
    std::string handle;
    std::string tagline;
    int ready_status = 0;  // 0 = not ready, 1 = ready
    
    std::string getDisplayName() const {
        if (!handle.empty() && !tagline.empty()) {
            return handle + "#" + tagline;
        }
        return handle.empty() ? username : handle;
    }
};

struct Lobby {
    std::string id;
    int64_t host_id = 0;
    std::string state;  // "waiting", "starting", "playing"
    int max_players = 6;
    std::string created_at;
    std::vector<LobbyMember> members;
    
    int getMemberCount() const { return static_cast<int>(members.size()); }
    bool isFull() const { return getMemberCount() >= max_players; }
    
    LobbyMember* findMember(int64_t userId) {
        for (auto& member : members) {
            if (member.user_id == userId) {
                return &member;
            }
        }
        return nullptr;
    }
};

// Singleton manager for social features
class SocialManager {
public:
    using Callback = std::function<void(bool success, const std::string& message)>;
    using UserCallback = std::function<void(const User& user)>;
    using FriendsCallback = std::function<void(const std::vector<Friend>& friends)>;
    using RequestsCallback = std::function<void(const std::vector<FriendRequest>& requests)>;
    using LobbyCallback = std::function<void(const Lobby& lobby)>;
    using LobbiesCallback = std::function<void(const std::vector<Lobby>& lobbies)>;
    
    static SocialManager& instance() {
        static SocialManager inst;
        return inst;
    }
    
    // User info
    void setCurrentUser(const User& user);
    const User& getCurrentUser() const { return currentUser; }
    bool isLoggedIn() const { return currentUser.id != 0; }
    
    // Handle management
    void checkHandleAvailability(const std::string& handle, Callback callback);
    void setHandle(const std::string& handle, const std::string& tagline, UserCallback callback, Callback errorCallback);
    
    // Friends
    void loadFriends(FriendsCallback callback);
    void loadPendingRequests(RequestsCallback callback);
    void sendFriendRequest(const std::string& handleOrUsername, Callback callback);
    void acceptFriendRequest(int64_t fromUserId, Callback callback);
    void declineFriendRequest(int64_t fromUserId, Callback callback);
    void removeFriend(int64_t friendId, Callback callback);
    
    // Lobbies
    void loadLobbies(LobbiesCallback callback);
    void createLobby(int maxPlayers, LobbyCallback callback, Callback errorCallback);
    void joinLobby(const std::string& lobbyId, LobbyCallback callback, Callback errorCallback);
    void leaveLobby(const std::string& lobbyId, Callback callback);
    void setReady(const std::string& lobbyId, bool ready, LobbyCallback callback, Callback errorCallback);
    void getCurrentLobby(Lobby& lobby) const;
    
    // Cached data
    const std::vector<Friend>& getFriends() const { return friends; }
    const std::vector<FriendRequest>& getPendingRequests() const { return pendingRequests; }
    const std::vector<Lobby>& getLobbies() const { return availableLobbies; }
    const Lobby* getJoinedLobby() const { return joinedLobby ? &(*joinedLobby) : nullptr; }
    
    // Refresh data
    void refreshAll();
    
private:
    SocialManager() = default;
    
    User currentUser;
    std::vector<Friend> friends;
    std::vector<FriendRequest> pendingRequests;
    std::vector<Lobby> availableLobbies;
    std::optional<Lobby> joinedLobby;
};
