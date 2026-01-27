# Client Integration Guide - Social Features

## Quick Start for Client Developers

This guide shows how to integrate the new username/handle and friend system into the game client.

## 1. Handle Setup Flow

### On User Login (OAuth)

```cpp
// After successful OAuth login
void OnLoginSuccess(const AuthResponse& response) {
    if (response.user.needs_handle_setup) {
        // Show handle selection screen
        ShowHandleSetupScreen(response.user.email);
    } else {
        // Continue to main menu
        ShowMainMenu();
    }
}
```

### Handle Selection UI

```cpp
class HandleSetupScreen {
private:
    std::string currentInput;
    bool isChecking = false;
    bool isAvailable = false;
    std::string message;
    
public:
    void OnHandleInput(const std::string& input) {
        currentInput = input;
        
        // Debounce checking (wait for user to stop typing)
        DebounceCheckAvailability(input, 500ms);
    }
    
    void CheckAvailability(const std::string& handle) {
        isChecking = true;
        
        // POST /username/check
        json request = {{"handle", handle}};
        HTTP::Post("http://coordinator:8888/username/check", request,
            [this](const json& response) {
                isChecking = false;
                isAvailable = response["available"];
                message = response["message"];
            });
    }
    
    void OnConfirm() {
        if (!isAvailable) return;
        
        // POST /username/set
        json request = {{"handle", currentInput}};
        HTTP::Post("http://coordinator:8888/username/set", request,
            [](const json& response) {
                if (response["success"]) {
                    // Update local user object
                    UpdateUser(response["user"]);
                    // Continue to main menu
                    ShowMainMenu();
                }
            }, authToken);
    }
    
    void Render() {
        ImGui::Begin("Choose Your Handle");
        
        ImGui::Text("Pick a unique username for your friends to find you");
        
        // Input field
        ImGui::InputText("Handle", &currentInput);
        
        // Validation message
        if (isChecking) {
            ImGui::TextColored({0.8f, 0.8f, 0.8f, 1.0f}, "Checking...");
        } else if (!message.empty()) {
            ImVec4 color = isAvailable ? 
                ImVec4{0.3f, 0.9f, 0.3f, 1.0f} : // Green
                ImVec4{0.9f, 0.3f, 0.3f, 1.0f};  // Red
            ImGui::TextColored(color, message.c_str());
        }
        
        // Confirm button
        ImGui::BeginDisabled(!isAvailable || currentInput.empty());
        if (ImGui::Button("Confirm")) {
            OnConfirm();
        }
        ImGui::EndDisabled();
        
        ImGui::End();
    }
};
```

## 2. Friend Search and Add

### Friend Search UI

```cpp
class FriendSearchScreen {
private:
    std::string searchInput;
    bool isSearching = false;
    std::string errorMessage;
    
public:
    void OnSendRequest() {
        isSearching = true;
        errorMessage.clear();
        
        // POST /friends/send
        json request = {{"friend_username", searchInput}};
        HTTP::Post("http://coordinator:8888/friends/send", request,
            [this](const json& response) {
                isSearching = false;
                if (response["success"]) {
                    // Show success notification
                    ShowNotification("Friend request sent!");
                    searchInput.clear();
                } else {
                    errorMessage = response["message"];
                }
            }, authToken);
    }
    
    void Render() {
        ImGui::Begin("Add Friend");
        
        ImGui::Text("Enter your friend's handle:");
        ImGui::InputText("##search", &searchInput);
        
        if (!errorMessage.empty()) {
            ImGui::TextColored({0.9f, 0.3f, 0.3f, 1.0f}, errorMessage.c_str());
        }
        
        ImGui::BeginDisabled(searchInput.empty() || isSearching);
        if (ImGui::Button("Send Request")) {
            OnSendRequest();
        }
        ImGui::EndDisabled();
        
        ImGui::End();
    }
};
```

## 3. Friends List Display

### Friend List UI

```cpp
class FriendsListScreen {
private:
    std::vector<Friend> friends;
    std::vector<FriendRequest> pendingRequests;
    
public:
    void LoadFriends() {
        // GET /friends/list
        HTTP::Get("http://coordinator:8888/friends/list",
            [this](const json& response) {
                if (response["success"]) {
                    friends = response["friends"];
                }
            }, authToken);
        
        // GET /friends/pending
        HTTP::Get("http://coordinator:8888/friends/pending",
            [this](const json& response) {
                if (response["success"]) {
                    pendingRequests = response["requests"];
                }
            }, authToken);
    }
    
    void OnAcceptRequest(int64_t fromUserId) {
        json request = {{"friend_id", fromUserId}};
        HTTP::Post("http://coordinator:8888/friends/accept", request,
            [this](const json& response) {
                if (response["success"]) {
                    LoadFriends(); // Refresh lists
                }
            }, authToken);
    }
    
    void OnRemoveFriend(int64_t friendId) {
        json request = {{"friend_id", friendId}};
        HTTP::Delete("http://coordinator:8888/friends/remove", request,
            [this](const json& response) {
                if (response["success"]) {
                    LoadFriends(); // Refresh lists
                }
            }, authToken);
    }
    
    void Render() {
        ImGui::Begin("Friends");
        
        // Pending Requests Section
        if (!pendingRequests.empty()) {
            ImGui::SeparatorText("Pending Requests");
            for (const auto& req : pendingRequests) {
                ImGui::PushID(req.id);
                ImGui::Text("%s wants to be friends", req.from_username.c_str());
                ImGui::SameLine();
                if (ImGui::Button("Accept")) {
                    OnAcceptRequest(req.from_user_id);
                }
                ImGui::SameLine();
                if (ImGui::Button("Decline")) {
                    OnDeclineRequest(req.from_user_id);
                }
                ImGui::PopID();
            }
            ImGui::Spacing();
        }
        
        // Friends List
        ImGui::SeparatorText("Friends");
        for (const auto& friend_ : friends) {
            ImGui::PushID(friend_.id);
            
            // Show online status indicator
            bool isOnline = CheckIfOnline(friend_.friend_id);
            ImVec4 statusColor = isOnline ? 
                ImVec4{0.3f, 0.9f, 0.3f, 1.0f} : 
                ImVec4{0.5f, 0.5f, 0.5f, 1.0f};
            ImGui::TextColored(statusColor, "●");
            
            ImGui::SameLine();
            ImGui::Text(friend_.friend_username.c_str());
            
            ImGui::SameLine(ImGui::GetWindowWidth() - 120);
            if (ImGui::Button("Invite")) {
                InviteToGame(friend_.friend_id);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                OnRemoveFriend(friend_.friend_id);
            }
            
            ImGui::PopID();
        }
        
        // Add friend button
        ImGui::Spacing();
        if (ImGui::Button("Add Friend")) {
            ShowFriendSearchScreen();
        }
        
        ImGui::End();
    }
};
```

## 4. Data Structures

### User Object

```cpp
struct User {
    int64_t id;
    std::string username;        // Internal username
    std::string handle;          // Display handle (empty if not set)
    std::string email;
    bool needs_handle_setup;
    std::string created_at;
    
    // Helper: Get display name (prefer handle over username)
    std::string GetDisplayName() const {
        return handle.empty() ? username : handle;
    }
};
```

### Friend Object

```cpp
struct Friend {
    int64_t id;
    int64_t user_id;
    int64_t friend_id;
    std::string status;          // "accepted"
    std::string friend_username;
};
```

### Friend Request Object

```cpp
struct FriendRequest {
    int64_t id;
    int64_t from_user_id;
    std::string from_username;
    std::string created_at;
};
```

## 5. HTTP Helper Functions

### Example HTTP Wrapper

```cpp
class HTTP {
public:
    using Callback = std::function<void(const json&)>;
    
    static void Get(const std::string& url, Callback callback, 
                    const std::string& token = "") {
        std::thread([url, callback, token]() {
            CURL* curl = curl_easy_init();
            struct curl_slist* headers = nullptr;
            
            if (!token.empty()) {
                headers = curl_slist_append(headers, 
                    ("Authorization: Bearer " + token).c_str());
            }
            
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            
            CURLcode res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                callback(json::parse(response));
            }
            
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }).detach();
    }
    
    static void Post(const std::string& url, const json& data, 
                     Callback callback, const std::string& token = "") {
        std::thread([url, data, callback, token]() {
            CURL* curl = curl_easy_init();
            struct curl_slist* headers = nullptr;
            
            headers = curl_slist_append(headers, "Content-Type: application/json");
            if (!token.empty()) {
                headers = curl_slist_append(headers, 
                    ("Authorization: Bearer " + token).c_str());
            }
            
            std::string payload = data.dump();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            
            CURLcode res = curl_easy_perform(curl);
            
            if (res == CURLE_OK) {
                callback(json::parse(response));
            }
            
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }).detach();
    }
    
private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, 
                                 std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};
```

## 6. Best Practices

### Handle Input Validation
- Validate on client side before sending (instant feedback)
- Show character count: "12/20 characters"
- Disable submit until valid
- Debounce availability checks (500ms delay)

### Error Handling
- Show user-friendly error messages
- Handle network timeouts gracefully
- Retry failed requests with exponential backoff

### UI/UX Tips
- **Real-time feedback**: Check availability as user types
- **Suggestions**: Pre-fill suggested handle for OAuth users
- **Visual indicators**: Green checkmark for available, red X for taken
- **Online status**: Show which friends are currently online
- **Notifications**: Alert user when friend requests are received
- **Confirmation**: Ask before removing friends

### Performance
- Cache friend lists locally
- Poll for pending requests every 30-60 seconds
- Use WebSocket for real-time friend status updates (future enhancement)

## 7. Example Integration Points

### Main Menu
```cpp
void MainMenuScreen::Render() {
    if (ImGui::Button("Singleplayer")) { ... }
    if (ImGui::Button("Multiplayer")) { ... }
    if (ImGui::Button("Friends")) {
        ShowFriendsScreen();
    }
    if (ImGui::Button("Settings")) { ... }
}
```

### Options Menu
```cpp
void OptionsScreen::Render() {
    // ... existing options ...
    
    ImGui::SeparatorText("Profile");
    ImGui::Text("Handle: %s", currentUser.GetDisplayName().c_str());
    if (ImGui::Button("Change Handle")) {
        ShowHandleChangeScreen();
    }
}
```

### Lobby System
```cpp
void LobbyScreen::OnInviteFriend(int64_t friendId) {
    // Future: Send lobby invite to friend
    json request = {
        {"friend_id", friendId},
        {"lobby_id", currentLobby.id}
    };
    HTTP::Post("http://coordinator:8888/lobby/invite", request,
        [](const json& response) {
            // Handle response
        }, authToken);
}
```

## 8. Testing Checklist

- [ ] Handle setup screen appears for new OAuth users
- [ ] Handle availability checking works
- [ ] Invalid handles show appropriate errors
- [ ] Friend search finds users by handle (case-insensitive)
- [ ] Friend requests send successfully
- [ ] Pending requests display correctly
- [ ] Accept/decline friend requests work
- [ ] Friend list updates after changes
- [ ] Remove friend confirmation works
- [ ] Display names use handles when available
- [ ] Network errors handled gracefully
- [ ] UI responsive during async operations

## 9. Configuration

Add to your config file:

```json
{
    "coordinator_url": "http://localhost:8888",
    "friend_request_poll_interval": 60,
    "handle_check_debounce_ms": 500
}
```

## Complete Example Flow

```cpp
// 1. User logs in with Google
AuthService::LoginWithGoogle([](const AuthResponse& response) {
    if (response.success) {
        authToken = response.token;
        currentUser = response.user;
        
        // 2. Check if handle setup needed
        if (currentUser.needs_handle_setup) {
            ShowHandleSetupScreen();
        } else {
            // 3. Load friends list
            FriendsService::LoadFriends([](const auto& friends) {
                // 4. Show main menu with friend count badge
                ShowMainMenu(friends.size());
            });
        }
    }
});

// 5. User clicks "Friends" button
void OnFriendsButtonClick() {
    ShowFriendsScreen();
    
    // Poll for new requests every minute
    StartFriendRequestPolling(60s);
}

// 6. User adds a friend
void OnAddFriendClick() {
    ShowFriendSearchDialog([](const std::string& handle) {
        FriendsService::SendRequest(handle, [](bool success) {
            if (success) {
                ShowNotification("Friend request sent!");
            }
        });
    });
}
```

## Support

For issues or questions, see:
- [SOCIAL_FEATURES.md](./SOCIAL_FEATURES.md) - Full documentation
- [API Reference](./API.md) - Complete API documentation
- Test scripts in `coordinator/` directory for examples
