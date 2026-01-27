#include "SocialManager.h"
#include "HttpClient.h"
#include "../utils/SimpleJson.h"
#include "../core/Settings.h"
#include <iostream>
#include <thread>

void SocialManager::setCurrentUser(const User& user) {
    currentUser = user;
    Settings::username = user.username;
    Settings::userID = user.id;
}

void SocialManager::checkHandleAvailability(const std::string& handle, Callback callback) {
    std::thread([handle, callback]() {
        SimpleJson request;
        request.set("handle", handle);
        
        std::string response = HttpClient::post(
            "http://localhost:8888/username/check",
            request.toString()
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        bool available = resp.getBool("available");
        std::string message = resp.getString("message");
        
        if (callback) {
            callback(success && available, message);
        }
    }).detach();
}

void SocialManager::setHandle(const std::string& handle, const std::string& tagline, UserCallback callback, Callback errorCallback) {
    std::thread([this, handle, tagline, callback, errorCallback]() {
        SimpleJson request;
        request.set("handle", handle);
        request.set("tagline", tagline);
        
        std::string response = HttpClient::post(
            "http://localhost:8888/username/set",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Parse user object (stored as JSON string)
            std::string userJsonStr = resp.get("user");
            if (!userJsonStr.empty()) {
                SimpleJson userJson = SimpleJson::parse(userJsonStr);
                currentUser.handle = userJson.getString("handle");
                currentUser.tagline = userJson.getString("tagline");
                currentUser.needs_handle_setup = false;
            }
            
            if (callback) {
                callback(currentUser);
            }
        } else {
            if (errorCallback) {
                errorCallback(false, resp.getString("message"));
            }
        }
    }).detach();
}

void SocialManager::loadFriends(FriendsCallback callback) {
    std::thread([this, callback]() {
        std::string response = HttpClient::get(
            "http://localhost:8888/friends/list",
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        if (resp.getBool("success")) {
            friends.clear();
            auto friendsArray = resp.getArray("friends");
            
            for (const auto& friendJson : friendsArray) {
                Friend f;
                f.id = friendJson.getInt("id");
                f.user_id = friendJson.getInt("user_id");
                f.friend_id = friendJson.getInt("friend_id");
                f.status = friendJson.getString("status");
                f.friend_username = friendJson.getString("friend_username");
                f.friend_handle = friendJson.getString("friend_handle");
                f.friend_tagline = friendJson.getString("friend_tagline");
                friends.push_back(f);
            }
            
            if (callback) {
                callback(friends);
            }
        }
    }).detach();
}

void SocialManager::loadPendingRequests(RequestsCallback callback) {
    std::thread([this, callback]() {
        std::string response = HttpClient::get(
            "http://localhost:8888/friends/pending",
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        if (resp.getBool("success")) {
            pendingRequests.clear();
            auto requestsArray = resp.getArray("requests");
            
            for (const auto& reqJson : requestsArray) {
                FriendRequest req;
                req.id = reqJson.getInt("id");
                req.from_user_id = reqJson.getInt("from_user_id");
                req.from_username = reqJson.getString("from_username");
                req.from_handle = reqJson.getString("from_handle");
                req.from_tagline = reqJson.getString("from_tagline");
                req.created_at = reqJson.getString("created_at");
                pendingRequests.push_back(req);
            }
            
            if (callback) {
                callback(pendingRequests);
            }
        }
    }).detach();
}

void SocialManager::sendFriendRequest(const std::string& handleOrUsername, Callback callback) {
    std::thread([handleOrUsername, callback]() {
        SimpleJson request;
        request.set("friend_username", handleOrUsername);
        
        std::string response = HttpClient::post(
            "http://localhost:8888/friends/send",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        std::string message = resp.getString("message");
        
        if (callback) {
            callback(success, message);
        }
    }).detach();
}

void SocialManager::acceptFriendRequest(int64_t fromUserId, Callback callback) {
    std::thread([this, fromUserId, callback]() {
        SimpleJson request;
        request.set("friend_id", std::to_string(fromUserId));
        
        std::string response = HttpClient::post(
            "http://localhost:8888/friends/accept",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Refresh friends and requests
            refreshAll();
        }
        
        if (callback) {
            callback(success, resp.getString("message"));
        }
    }).detach();
}

void SocialManager::declineFriendRequest(int64_t fromUserId, Callback callback) {
    std::thread([this, fromUserId, callback]() {
        SimpleJson request;
        request.set("friend_id", std::to_string(fromUserId));
        
        std::string response = HttpClient::post(
            "http://localhost:8888/friends/decline",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Refresh requests
            loadPendingRequests(nullptr);
        }
        
        if (callback) {
            callback(success, resp.getString("message"));
        }
    }).detach();
}

void SocialManager::removeFriend(int64_t friendId, Callback callback) {
    std::thread([this, friendId, callback]() {
        SimpleJson request;
        request.set("friend_id", std::to_string(friendId));
        
        // Using POST instead of DELETE for simplicity with HttpClient
        std::string response = HttpClient::post(
            "http://localhost:8888/friends/remove",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Refresh friends list
            loadFriends(nullptr);
        }
        
        if (callback) {
            callback(success, resp.getString("message"));
        }
    }).detach();
}

void SocialManager::refreshAll() {
    loadFriends(nullptr);
    loadPendingRequests(nullptr);
}

void SocialManager::loadLobbies(LobbiesCallback callback) {
    std::thread([this, callback]() {
        std::string response = HttpClient::get(
            "http://localhost:8888/lobby/list",
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        availableLobbies.clear();
        if (success) {
            auto lobbiesArray = resp.getArray("lobbies");
            for (const auto& lobbyJson : lobbiesArray) {
                Lobby lobby;
                lobby.id = lobbyJson.getString("id");
                lobby.host_id = std::stoll(lobbyJson.getString("host_id"));
                lobby.state = lobbyJson.getString("state");
                lobby.max_players = lobbyJson.getInt("max_players");
                lobby.created_at = lobbyJson.getString("created_at");
                
                auto membersArray = lobbyJson.getArray("members");
                for (const auto& memberJson : membersArray) {
                    LobbyMember member;
                    member.user_id = std::stoll(memberJson.getString("user_id"));
                    member.username = memberJson.getString("username");
                    member.handle = memberJson.getString("handle");
                    member.tagline = memberJson.getString("tagline");
                    member.ready_status = memberJson.getInt("ready_status");
                    lobby.members.push_back(member);
                }
                
                availableLobbies.push_back(lobby);
            }
        }
        
        if (callback) {
            callback(availableLobbies);
        }
    }).detach();
}

void SocialManager::createLobby(int maxPlayers, LobbyCallback callback, Callback errorCallback) {
    std::thread([this, maxPlayers, callback, errorCallback]() {
        SimpleJson request;
        request.set("max_players", maxPlayers);
        
        std::string response = HttpClient::post(
            "http://localhost:8888/lobby/create",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Extract lobby object from response
            std::string lobbyStr = resp.get("lobby");
            SimpleJson lobbyJson = SimpleJson::parse(lobbyStr);
            Lobby lobby;
            lobby.id = lobbyJson.getString("id");
            lobby.host_id = std::stoll(lobbyJson.getString("host_id"));
            lobby.state = lobbyJson.getString("state");
            lobby.max_players = lobbyJson.getInt("max_players");
            lobby.created_at = lobbyJson.getString("created_at");
            
            auto membersArray = lobbyJson.getArray("members");
            for (const auto& memberJson : membersArray) {
                LobbyMember member;
                member.user_id = std::stoll(memberJson.getString("user_id"));
                member.username = memberJson.getString("username");
                member.handle = memberJson.getString("handle");
                member.tagline = memberJson.getString("tagline");
                member.ready_status = memberJson.getInt("ready_status");
                lobby.members.push_back(member);
            }
            
            joinedLobby = lobby;
            if (callback) {
                callback(lobby);
            }
        } else {
            if (errorCallback) {
                errorCallback(false, resp.getString("message"));
            }
        }
    }).detach();
}

void SocialManager::joinLobby(const std::string& lobbyId, LobbyCallback callback, Callback errorCallback) {
    std::thread([this, lobbyId, callback, errorCallback]() {
        SimpleJson request;
        request.set("lobby_id", lobbyId);
        
        std::string response = HttpClient::post(
            "http://localhost:8888/lobby/join",
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Extract lobby object from response
            std::string lobbyStr = resp.get("lobby");
            SimpleJson lobbyJson = SimpleJson::parse(lobbyStr);
            Lobby lobby;
            lobby.id = lobbyJson.getString("id");
            lobby.host_id = std::stoll(lobbyJson.getString("host_id"));
            lobby.state = lobbyJson.getString("state");
            lobby.max_players = lobbyJson.getInt("max_players");
            lobby.created_at = lobbyJson.getString("created_at");
            
            auto membersArray = lobbyJson.getArray("members");
            for (const auto& memberJson : membersArray) {
                LobbyMember member;
                member.user_id = std::stoll(memberJson.getString("user_id"));
                member.username = memberJson.getString("username");
                member.handle = memberJson.getString("handle");
                member.tagline = memberJson.getString("tagline");
                member.ready_status = memberJson.getInt("ready_status");
                lobby.members.push_back(member);
            }
            
            joinedLobby = lobby;
            if (callback) {
                callback(lobby);
            }
        } else {
            if (errorCallback) {
                errorCallback(false, resp.getString("message"));
            }
        }
    }).detach();
}

void SocialManager::leaveLobby(const std::string& lobbyId, Callback callback) {
    std::thread([this, lobbyId, callback]() {
        std::string url = "http://localhost:8888/lobby/leave?lobby_id=" + lobbyId;
        
        std::string response = HttpClient::post(
            url,
            "{}",
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            joinedLobby = std::nullopt;
        }
        
        if (callback) {
            callback(success, resp.getString("message"));
        }
    }).detach();
}

void SocialManager::setReady(const std::string& lobbyId, bool ready, LobbyCallback callback, Callback errorCallback) {
    std::thread([this, lobbyId, ready, callback, errorCallback]() {
        SimpleJson request;
        request.set("ready", ready);
        
        std::string url = "http://localhost:8888/lobby/ready?lobby_id=" + lobbyId;
        
        std::string response = HttpClient::post(
            url,
            request.toString(),
            Settings::authToken
        );
        
        SimpleJson resp = SimpleJson::parse(response);
        bool success = resp.getBool("success");
        
        if (success) {
            // Extract lobby object from response
            std::string lobbyStr = resp.get("lobby");
            SimpleJson lobbyJson = SimpleJson::parse(lobbyStr);
            Lobby lobby;
            lobby.id = lobbyJson.getString("id");
            lobby.host_id = std::stoll(lobbyJson.getString("host_id"));
            lobby.state = lobbyJson.getString("state");
            lobby.max_players = lobbyJson.getInt("max_players");
            lobby.created_at = lobbyJson.getString("created_at");
            
            auto membersArray = lobbyJson.getArray("members");
            for (const auto& memberJson : membersArray) {
                LobbyMember member;
                member.user_id = std::stoll(memberJson.getString("user_id"));
                member.username = memberJson.getString("username");
                member.handle = memberJson.getString("handle");
                member.tagline = memberJson.getString("tagline");
                member.ready_status = memberJson.getInt("ready_status");
                lobby.members.push_back(member);
            }
            
            joinedLobby = lobby;
            if (callback) {
                callback(lobby);
            }
        } else {
            if (errorCallback) {
                errorCallback(false, resp.getString("message"));
            }
        }
    }).detach();
}

void SocialManager::getCurrentLobby(Lobby& lobby) const {
    if (joinedLobby) {
        lobby = *joinedLobby;
    }
}
