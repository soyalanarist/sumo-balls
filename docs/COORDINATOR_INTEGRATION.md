# Integrating Coordinator with C++ Game Client and Server

This document describes how to integrate the Go coordinator with your existing C++ game client and server.

## Architecture Overview

```
┌──────────────┐         HTTP/REST         ┌────────────┐
│   C++ Client │◄──────────────────────────┤ Go         │
│   (SFML)     │     /enqueue, /queue/*    │ Coordinator│
└──────────────┘                            └────────────┘
       │                                            ▲
       │                                            │
       │  ENet UDP (game protocol)        HTTP/REST │
       │                          /register, /heartbeat
       ▼                                            │
┌──────────────────┐                       ┌────────────┐
│  C++ Game Server │◄──────────────────────┘ Registry   │
│  (Authoritative) │                                    │
│  Port 9999       │                                    │
└──────────────────┘
```

## Step 1: Build Coordinator

```bash
cd coordinator
go build -o coordinator-bin main.go
```

Run it:
```bash
./coordinator-bin
# Output: [Coordinator] Starting on :8888
```

Verify it's working:
```bash
curl -X POST http://localhost:8888/enqueue \
  -H "Content-Type: application/json" \
  -d '{"player_id":"test1","mode":"matchmaking","region":"us-west"}'
```

## Step 2: Modify C++ Game Server (server_main.cpp)

### Add HTTP Client Library

You need an HTTP library for C++ (pick one):
- **curl** (recommended for simplicity)
- **Beast** (Boost.Beast, if you like C++ idioms)
- **restclient-cpp** (lightweight)

Install curl:
```bash
sudo apt install libcurl4-openssl-dev
```

### Server Registration & Heartbeat (src/server_main.cpp)

Add to your server initialization:

```cpp
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper function to call coordinator
std::string coordinatorPost(const std::string& endpoint, const json& body) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    std::string url = "http://localhost:8888" + endpoint;
    std::string jsonStr = body.dump();
    std::string responseData;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return responseData;
}

// In your server main loop, after starting:
void serverMain() {
    // ... existing code ...
    
    std::string serverId = "server_" + generateUniqueId();
    std::string host = "192.168.1.100";  // Your server's IP
    int port = 9999;                      // Your game server port
    int maxPlayers = 4;
    
    // Register with coordinator
    json registerReq = {
        {"server_id", serverId},
        {"host", host},
        {"port", port},
        {"max_players", maxPlayers}
    };
    
    std::string resp = coordinatorPost("/server/register", registerReq);
    std::cout << "[Server] Registered with coordinator: " << resp << std::endl;
    
    // Heartbeat loop (every 5 seconds)
    auto lastHeartbeat = std::chrono::steady_clock::now();
    
    while (serverRunning) {
        // ... game logic ...
        
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat).count() >= 5) {
            json heartbeatReq = {{"server_id", serverId}};
            coordinatorPost("/server/heartbeat", heartbeatReq);
            lastHeartbeat = now;
        }
    }
}
```

## Step 3: Modify C++ Game Client (src/screens/GameScreen.cpp)

### Menu Screen Changes

Modify menu to show "Matchmaking" vs "Singleplayer":

```cpp
// In your menu screen:
enum class GameMode {
    SINGLEPLAYER,
    MATCHMAKING
};

// When player clicks "Matchmaking":
void onMatchmakingButtonClicked() {
    gameMode = GameMode::MATCHMAKING;
    enqueuePlayer();
    showQueueUI = true;
}

// Enqueue function
void enqueuePlayer() {
    CURL* curl = curl_easy_init();
    
    json enqueueReq = {
        {"player_id", playerId},
        {"mode", "matchmaking"},
        {"region", "us-west"}
    };
    
    std::string jsonStr = enqueueReq.dump();
    
    // POST to coordinator
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8888/enqueue");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
    // ... (setup headers, response handler, etc.) ...
    
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}
```

### Queue Polling Loop

In your GameScreen update loop:

```cpp
void GameScreen::update(float deltaTime) {
    // ... existing update code ...
    
    if (gameMode == GameMode::MATCHMAKING && showQueueUI) {
        // Poll coordinator every 1 second
        static float pollTimer = 0;
        pollTimer += deltaTime;
        
        if (pollTimer >= 1.0f) {
            pollTimer = 0;
            checkQueueStatus();
        }
    }
}

void GameScreen::checkQueueStatus() {
    CURL* curl = curl_easy_init();
    
    std::string url = "http://localhost:8888/queue/status?player_id=" + playerId;
    std::string responseData;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
    
    curl_easy_perform(curl);
    
    try {
        json response = json::parse(responseData);
        
        if (response["status"] == "match_found") {
            // Match ready! Connect to game server
            std::string serverHost = response["server_host"];
            int serverPort = response["server_port"];
            std::string token = response["token"];
            
            connectToGameServer(serverHost, serverPort, token);
            showQueueUI = false;
        } else if (response["status"] == "queued") {
            // Still waiting
            int64_t elapsedMs = response["elapsed_ms"];
            queueElapsedSeconds = elapsedMs / 1000;
            // Update UI with time elapsed
        }
    } catch (std::exception& e) {
        std::cerr << "Failed to parse queue status: " << e.what() << std::endl;
    }
    
    curl_easy_cleanup(curl);
}

void GameScreen::connectToGameServer(const std::string& host, int port, const std::string& token) {
    // Store token as session token
    sessionToken = token;
    
    // Proceed with existing ENet connection logic
    netClient->connect(host, port);
    
    // When sending JoinRequest, include token in first message
    // (modify NetClient to send token as additional field)
}
```

## Step 4: Modify Game Server Token Validation

In [src/server_main.cpp](src/server_main.cpp), when accepting a client join:

```cpp
// In your JoinRequest handler:
void handleJoinRequest(const JoinRequest& req) {
    std::string token = req.sessionToken;  // Add to JoinRequest struct
    
    // For MVP, just validate that token exists
    // (In production, validate HMAC signature)
    if (token.empty()) {
        std::cerr << "Client provided no session token" << std::endl;
        return;  // Reject join
    }
    
    // Extract playerId from token (could be embedded or looked up)
    // For now, trust coordinator that token is valid
    
    acceptPlayerJoin(req);
}
```

## Step 5: Update Network Protocol

In [src/network/NetProtocol.h](src/network/NetProtocol.h), add token field:

```cpp
struct JoinRequest {
    uint32_t playerId;
    std::string playerName;
    std::string sessionToken;  // NEW: Coordinator-issued token
    
    // Serialization: encode sessionToken as length + string bytes
};
```

## Testing Integration

### Test 1: Local Matchmaking (2 clients on same machine)

Terminal 1:
```bash
cd /home/soyal/sumo-balls/coordinator
./coordinator-bin
# Output: [Coordinator] Starting on :8888
```

Terminal 2:
```bash
cd /home/soyal/sumo-balls/build
./sumo_balls
# Click "Matchmaking" button
```

Terminal 3:
```bash
cd /home/soyal/sumo-balls/build
./sumo_balls
# Click "Matchmaking" button
```

Expected: After ~2 seconds, both clients get "Match found" notification and connect to each other.

### Test 2: Custom Server

Terminal 1:
```bash
cd /home/soyal/sumo-balls/coordinator
./coordinator-bin
```

Terminal 2:
```bash
cd /home/soyal/sumo-balls/build
./sumo_balls_server
# Output: [Server] Registered with coordinator
```

Terminal 3-4: Run two clients as above.

Expected: Server handles both clients, broadcasts snapshots every 30ms.

## Debugging

**Coordinator logs:**
- Check if coordinator is listening: `curl http://localhost:8888/matches`
- Check active matches: `curl http://localhost:8888/matches | python3 -m json.tool`

**Client HTTP calls:**
- Add logging to curl calls: `curl -v http://localhost:8888/...`
- Check network with: `netstat -an | grep 8888`

**Server heartbeats:**
- Monitor coordinator heartbeat endpoint: `while true; do curl http://localhost:8888/matches; sleep 1; done`

## Future Enhancements

1. **Token Signing**: Use HMAC-SHA256 to sign tokens so server can validate without coordinator
2. **Token Fields**: Embed playerId, matchId, expiry in token structure
3. **Server Backfill**: Add players to matches in progress
4. **Region Latency**: Route clients to nearest server based on ping
5. **Persistent Storage**: Move from in-memory maps to SQLite/PostgreSQL
6. **Distributed Coordinator**: Use multiple Go coordinators behind a load balancer (Redis for shared state)
7. **Observability**: Add Prometheus metrics endpoint for monitoring queue length, match creation rate, etc.
