package main

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"
)

// Helper constants
const (
	timeHour = time.Hour
)

// Helper functions
func timeNow() time.Time {
	return time.Now()
}

func respondJSON(w http.ResponseWriter, data interface{}, statusCode int) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(statusCode)
	json.NewEncoder(w).Encode(data)
}

// ============================================================================
// TYPES & DATA STRUCTURES
// ============================================================================

// QueuedPlayer represents a player waiting for a match.
// In C++, this would be a struct with member variables:
//   struct QueuedPlayer {
//       std::string playerId;
//       uint64_t enqueueTime;
//   };
type QueuedPlayer struct {
	PlayerID   string `json:"player_id"`
	EnqueuedAt int64  `json:"enqueued_at"` // Unix milliseconds
}

// GameServer represents a registered game server in the pool.
// In C++: struct GameServer { std::string id; std::string host; uint16_t port; ... };
type GameServer struct {
	ServerID    string
	Host        string
	Port        int
	MaxPlayers  int
	CurrentLoad int
	LastHeartbeat time.Time
}

// Match represents an active match session.
// In C++: struct Match { std::string id; std::vector<std::string> playerIds; ... };
type Match struct {
	MatchID    string
	ServerID   string
	PlayerIDs  []string
	Token      string // Secure token clients use to join
	CreatedAt  time.Time
}

// QueueRequest is the JSON payload a client sends to enqueue.
// In C++, this would be deserialized from JSON manually or with a library.
type QueueRequest struct {
	PlayerID string `json:"player_id"`
	Mode     string `json:"mode"` // "matchmaking" or "singleplayer"
	Region   string `json:"region"`
}

// QueueResponse is sent back to the client after enqueuing.
type QueueResponse struct {
	QueueID    string `json:"queue_id"`
	Status     string `json:"status"` // "queued", "error", etc.
	Message    string `json:"message,omitempty"`
	ElapsedMs  int64  `json:"elapsed_ms,omitempty"`
	EstWaitMs  int64  `json:"est_wait_ms,omitempty"`
}

// MatchFoundResponse is sent when a match is ready.
type MatchFoundResponse struct {
	Status       string `json:"status"` // "match_found"
	ServerHost   string `json:"server_host"`
	ServerPort   int    `json:"server_port"`
	Token        string `json:"token"`
	MatchID      string `json:"match_id"`
	ServerTimeMs int64  `json:"server_time_ms"`
}

// ServerRegisterRequest is sent by game servers to register with coordinator.
type ServerRegisterRequest struct {
	ServerID   string `json:"server_id"`
	Host       string `json:"host"`
	Port       int    `json:"port"`
	MaxPlayers int    `json:"max_players"`
}

// ============================================================================
// COORDINATOR STATE
// ============================================================================

// Coordinator holds all mutable state.
// In C++, this would be a singleton class with private member variables and mutex locks.
// Go uses a struct with a sync.Mutex (similar to std::mutex in C++).
type Coordinator struct {
	mu              sync.Mutex            // Protects all fields below (like std::lock_guard in C++)
	queuedPlayers   []QueuedPlayer        // FIFO queue of waiting players
	gameServers     map[string]*GameServer // Pool of available game servers
	activeMatches   map[string]*Match     // Ongoing matches by matchID
	playerQueues    map[string]string     // Map: playerID -> queueID for lookups
	nextMatchNumber int64                 // Counter for unique match IDs
}

// NewCoordinator creates a new coordinator instance.
// In C++: static Coordinator* instance = nullptr; // singleton pattern
// In Go, we just return a new struct. No garbage collection concerns like C++.
func NewCoordinator() *Coordinator {
	return &Coordinator{
		gameServers:   make(map[string]*GameServer),
		activeMatches: make(map[string]*Match),
		playerQueues:  make(map[string]string),
	}
}

// ============================================================================
// QUEUE OPERATIONS
// ============================================================================

// Enqueue adds a player to the queue.
// In C++: void Coordinator::enqueue(const std::string& playerId) { ... }
// In Go, we use a receiver (c *Coordinator) which is like 'this' pointer in C++.
func (c *Coordinator) Enqueue(playerID string) (queueID string) {
	// c.mu.Lock() / Unlock() is like RAII scoped_lock in C++.
	// Holding the lock ensures no race conditions (thread-safe).
	c.mu.Lock()
	defer c.mu.Unlock() // defer is like std::lock_guard destructor—auto-unlocks

	// Remove player if already queued (idempotent)
	if _, exists := c.playerQueues[playerID]; exists {
		for i, p := range c.queuedPlayers {
			if p.PlayerID == playerID {
				c.queuedPlayers = append(c.queuedPlayers[:i], c.queuedPlayers[i+1:]...)
				break
			}
		}
	}

	// Add player to queue
	queueID = fmt.Sprintf("q_%s", playerID)
	c.queuedPlayers = append(c.queuedPlayers, QueuedPlayer{
		PlayerID:   playerID,
		EnqueuedAt: timeNowMs(),
	})
	c.playerQueues[playerID] = queueID

	log.Printf("[Coordinator] Player %s enqueued (total: %d)\n", playerID, len(c.queuedPlayers))
	return queueID
}

// Dequeue removes a player from the queue.
// In C++: void Coordinator::dequeue(const std::string& playerId) { ... }
func (c *Coordinator) Dequeue(playerID string) {
	c.mu.Lock()
	defer c.mu.Unlock()

	for i, p := range c.queuedPlayers {
		if p.PlayerID == playerID {
			c.queuedPlayers = append(c.queuedPlayers[:i], c.queuedPlayers[i+1:]...)
			delete(c.playerQueues, playerID)
			log.Printf("[Coordinator] Player %s dequeued (total: %d)\n", playerID, len(c.queuedPlayers))
			return
		}
	}
}

// GetQueueStatus returns wait time and queue position.
// In C++: struct QueueStatus { int64_t elapsedMs; int64_t estWaitMs; };
//         QueueStatus Coordinator::getQueueStatus(const std::string& playerId) { ... }
func (c *Coordinator) GetQueueStatus(playerID string) (elapsedMs int64, estWaitMs int64) {
	c.mu.Lock()
	defer c.mu.Unlock()

	for _, p := range c.queuedPlayers {
		if p.PlayerID == playerID {
			elapsed := timeNowMs() - p.EnqueuedAt
			// Simple heuristic: est wait = elapsed + (queue length * time per player)
			// Assumes avg 3 seconds per match creation
			est := int64(len(c.queuedPlayers)) * 3000
			return elapsed, est
		}
	}
	return -1, -1 // Not queued
}

// ============================================================================
// MATCH CREATION & TOKEN GENERATION
// ============================================================================

// CreateMatch pulls players from queue and assigns them to a server.
// In C++: Match Coordinator::createMatch(const std::vector<std::string>& playerIds) { ... }
// This is the core matchmaking logic.
func (c *Coordinator) CreateMatch(playerIDs []string) (*Match, error) {
	c.mu.Lock()
	defer c.mu.Unlock()

	// Find least-loaded server with capacity
	var bestServer *GameServer
	for _, server := range c.gameServers {
		if server.CurrentLoad+len(playerIDs) <= server.MaxPlayers {
			if bestServer == nil || server.CurrentLoad < bestServer.CurrentLoad {
				bestServer = server
			}
		}
	}

	if bestServer == nil {
		return nil, fmt.Errorf("no available game servers")
	}

	// Generate match ID and secure token
	c.nextMatchNumber++
	matchID := fmt.Sprintf("match_%d", c.nextMatchNumber)
	token := generateToken()

	// Create match object
	match := &Match{
		MatchID:   matchID,
		ServerID:  bestServer.ServerID,
		PlayerIDs: playerIDs,
		Token:     token,
		CreatedAt: time.Now(),
	}

	// Store match and update server load
	c.activeMatches[matchID] = match
	bestServer.CurrentLoad += len(playerIDs)

	// Remove players from queue
	for _, playerID := range playerIDs {
		for i, p := range c.queuedPlayers {
			if p.PlayerID == playerID {
				c.queuedPlayers = append(c.queuedPlayers[:i], c.queuedPlayers[i+1:]...)
				delete(c.playerQueues, playerID)
				break
			}
		}
	}

	log.Printf("[Coordinator] Match created: %s on server %s with %d players\n", matchID, bestServer.ServerID, len(playerIDs))
	return match, nil
}

// generateToken creates a cryptographically secure token.
// In C++: std::string generateToken() { unsigned char buf[16]; RAND_bytes(buf, 16); ... }
// Go's crypto/rand is like OpenSSL's RAND_bytes.
func generateToken() string {
	b := make([]byte, 16)
	_, err := rand.Read(b)
	if err != nil {
		log.Fatalf("Failed to generate token: %v", err)
	}
	return hex.EncodeToString(b)
}

// ============================================================================
// SERVER POOL MANAGEMENT
// ============================================================================

// RegisterServer adds a game server to the pool.
// In C++: void Coordinator::registerServer(const ServerRegisterRequest& req) { ... }
func (c *Coordinator) RegisterServer(req ServerRegisterRequest) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	c.gameServers[req.ServerID] = &GameServer{
		ServerID:      req.ServerID,
		Host:          req.Host,
		Port:          req.Port,
		MaxPlayers:    req.MaxPlayers,
		CurrentLoad:   0,
		LastHeartbeat: time.Now(),
	}

	log.Printf("[Coordinator] Server registered: %s (%s:%d, max %d players)\n", req.ServerID, req.Host, req.Port, req.MaxPlayers)
	return nil
}

// Heartbeat updates server's last-seen timestamp.
// In C++: void Coordinator::heartbeat(const std::string& serverId) { ... }
// Servers should ping this every 5-10 seconds; if no heartbeat in 30 seconds, mark server dead.
func (c *Coordinator) Heartbeat(serverID string) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	server, exists := c.gameServers[serverID]
	if !exists {
		return fmt.Errorf("server not found: %s", serverID)
	}

	server.LastHeartbeat = time.Now()
	return nil
}

// ============================================================================
// PERIODIC MATCHMAKING LOOP
// ============================================================================

// MatchmakingLoop runs periodically to pull players from queue and create matches.
// In C++: void Coordinator::matchmakingLoop() { ... }
// Go's goroutines (go-routines) are like lightweight threads, but simpler.
// We use a ticker (like an alarm clock) to run every 2 seconds.
func (c *Coordinator) MatchmakingLoop(tickInterval time.Duration) {
	// time.NewTicker is like setting up a periodic timer.
	// In C++, you'd use std::thread + sleep_for in a loop.
	ticker := time.NewTicker(tickInterval)
	defer ticker.Stop()

	for {
		<-ticker.C // Wait for tick (blocks until timer fires)

		c.mu.Lock()
		// Try to create matches for groups of 2-4 players
		// Simple logic: if we have 2+ players, make a match.
		minPlayers := 2
		for len(c.queuedPlayers) >= minPlayers {
			// Take first N queued players
			playersToMatch := make([]string, 0, minPlayers)
			for i := 0; i < minPlayers && i < len(c.queuedPlayers); i++ {
				playersToMatch = append(playersToMatch, c.queuedPlayers[i].PlayerID)
			}

			c.mu.Unlock()

			// Create match (unlock first to avoid deadlock since CreateMatch also locks)
			_, err := c.CreateMatch(playersToMatch)
			if err != nil {
				log.Printf("[Coordinator] Failed to create match: %v\n", err)
				c.mu.Lock()
				break
			}

			c.mu.Lock()
		}
		c.mu.Unlock()

		// Log queue stats
		qlen := len(c.queuedPlayers)
		if qlen > 0 {
			log.Printf("[Coordinator] Queue status: %d players waiting\n", qlen)
		}
	}
}

// ============================================================================
// HTTP HANDLERS (REST API endpoints)
// ============================================================================

// handleEnqueue handles POST /enqueue
// In C++, this would be a handler function registered with a web framework.
// Go uses handler functions with signature: func(w http.ResponseWriter, r *http.Request)
func (c *Coordinator) handleEnqueue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req QueueRequest
	// json.NewDecoder is like parsing JSON in C++ with rapidjson or nlohmann::json
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request", http.StatusBadRequest)
		return
	}

	queueID := c.Enqueue(req.PlayerID)
	resp := QueueResponse{
		QueueID: queueID,
		Status:  "queued",
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

// handleStatus handles GET /queue/status/:playerID
func (c *Coordinator) handleStatus(w http.ResponseWriter, r *http.Request) {
	playerID := r.URL.Query().Get("player_id")
	if playerID == "" {
		http.Error(w, "Missing player_id", http.StatusBadRequest)
		return
	}

	elapsed, estWait := c.GetQueueStatus(playerID)
	if elapsed == -1 {
		http.Error(w, "Player not in queue", http.StatusNotFound)
		return
	}

	resp := QueueResponse{
		Status:   "queued",
		ElapsedMs: elapsed,
		EstWaitMs: estWait,
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

// handleCancel handles POST /cancel
func (c *Coordinator) handleCancel(w http.ResponseWriter, r *http.Request) {
	var req struct {
		PlayerID string `json:"player_id"`
	}

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request", http.StatusBadRequest)
		return
	}

	c.Dequeue(req.PlayerID)

	resp := map[string]string{"status": "canceled"}
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

// handleRegisterServer handles POST /server/register
func (c *Coordinator) handleRegisterServer(w http.ResponseWriter, r *http.Request) {
	var req ServerRegisterRequest

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request", http.StatusBadRequest)
		return
	}

	if err := c.RegisterServer(req); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	resp := map[string]string{"status": "registered"}
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

// handleHeartbeat handles POST /server/heartbeat
func (c *Coordinator) handleHeartbeat(w http.ResponseWriter, r *http.Request) {
	var req struct {
		ServerID string `json:"server_id"`
	}

	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid request", http.StatusBadRequest)
		return
	}

	if err := c.Heartbeat(req.ServerID); err != nil {
		http.Error(w, err.Error(), http.StatusNotFound)
		return
	}

	resp := map[string]string{"status": "heartbeat_received"}
	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}

// handleMatches handles GET /matches to list active matches (for debugging)
func (c *Coordinator) handleMatches(w http.ResponseWriter, r *http.Request) {
	c.mu.Lock()
	defer c.mu.Unlock()

	matches := make([]map[string]interface{}, 0)
	for _, match := range c.activeMatches {
		matches = append(matches, map[string]interface{}{
			"match_id":  match.MatchID,
			"server_id": match.ServerID,
			"players":   match.PlayerIDs,
			"created":   match.CreatedAt,
		})
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(matches)
}

// ============================================================================
// UTILITIES
// ============================================================================

// timeNowMs returns current time in milliseconds since epoch.
// In C++: auto now = std::chrono::system_clock::now();
//         auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(...);
func timeNowMs() int64 {
	return time.Now().UnixNano() / 1e6
}

// ============================================================================
// MAIN
// ============================================================================

func main() {
	// Initialize database
	db, err := NewDatabase("coordinator.db")
	if err != nil {
		log.Fatalf("[Fatal] Failed to initialize database: %v", err)
	}
	defer db.Close()

	// Initialize services
	authService := NewAuthService(db)
	friendsService := NewFriendsService(db)
	lobbyService := NewLobbyService(db)

	// Create coordinator instance (for legacy matchmaking endpoints)
	coord := NewCoordinator()

	// Start matchmaking loop in a separate goroutine (async, like std::thread)
	// This runs in the background, checking queue every 2 seconds
	go coord.MatchmakingLoop(2 * time.Second)

	// Auth endpoints (public - no auth required)
	http.HandleFunc("/auth/register", authService.handleRegister)
	http.HandleFunc("/auth/login", authService.handleLogin)
	http.HandleFunc("/auth/logout", authService.handleLogout)
	http.HandleFunc("/auth/me", authService.authMiddleware(authService.handleMe))

	// Google OAuth endpoints
	http.HandleFunc("/auth/google/init", authService.handleGoogleInit)
	http.HandleFunc("/auth/google/callback", authService.handleGoogleCallback)
	http.HandleFunc("/auth/google/status", authService.handleGoogleStatus)

	// Friend endpoints (require authentication)
	http.HandleFunc("/friends/send", authService.authMiddleware(friendsService.handleSendRequest))
	http.HandleFunc("/friends/accept", authService.authMiddleware(friendsService.handleAcceptRequest))
	http.HandleFunc("/friends/remove", authService.authMiddleware(friendsService.handleRemoveFriend))
	http.HandleFunc("/friends/list", authService.authMiddleware(friendsService.handleListFriends))
	http.HandleFunc("/friends/pending", authService.authMiddleware(friendsService.handlePendingRequests))

	// Lobby endpoints (require authentication)
	http.HandleFunc("/lobby/create", authService.authMiddleware(lobbyService.handleCreateLobby))
	http.HandleFunc("/lobby/join", authService.authMiddleware(lobbyService.handleJoinLobby))
	http.HandleFunc("/lobby/leave", authService.authMiddleware(lobbyService.handleLeaveLobby))
	http.HandleFunc("/lobby/ready", authService.authMiddleware(lobbyService.handleSetReady))
	http.HandleFunc("/lobby/get", authService.authMiddleware(lobbyService.handleGetLobby))

	// Legacy matchmaking endpoints (for backward compatibility)
	http.HandleFunc("/enqueue", coord.handleEnqueue)
	http.HandleFunc("/cancel", coord.handleCancel)
	http.HandleFunc("/queue/status", coord.handleStatus)
	http.HandleFunc("/server/register", coord.handleRegisterServer)
	http.HandleFunc("/server/heartbeat", coord.handleHeartbeat)
	http.HandleFunc("/matches", coord.handleMatches) // Debug endpoint

	// Start HTTP server on port 8888
	port := ":8888"
	log.Printf("[Coordinator] Starting with auth, friends, and lobby system on %s\n", port)
	log.Printf("[Coordinator] Database: coordinator.db\n")
	if err := http.ListenAndServe(port, nil); err != nil {
		log.Fatalf("Server error: %v", err)
	}
}
