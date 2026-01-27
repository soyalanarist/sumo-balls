package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
)

type LobbyService struct {
	db            *Database
	mu            sync.Mutex
	activeLobbies map[string]*Lobby // In-memory cache of active lobbies
	lobbyCounter  int64
}

func NewLobbyService(db *Database) *LobbyService {
	return &LobbyService{
		db:            db,
		activeLobbies: make(map[string]*Lobby),
	}
}

// Create lobby
func (l *LobbyService) handleCreateLobby(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req CreateLobbyRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		// Use default max players
		req.MaxPlayers = 6
	}

	if req.MaxPlayers <= 0 || req.MaxPlayers > 6 {
		req.MaxPlayers = 6
	}

	l.mu.Lock()
	l.lobbyCounter++
	lobbyID := fmt.Sprintf("lobby_%d", l.lobbyCounter)
	l.mu.Unlock()

	// Create lobby in database
	if err := l.db.CreateLobby(lobbyID, user.ID); err != nil {
		log.Printf("[Lobby] Create lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to create lobby"}, http.StatusInternalServerError)
		return
	}

	// Host automatically joins
	if err := l.db.JoinLobby(lobbyID, user.ID); err != nil {
		log.Printf("[Lobby] Auto-join error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to join lobby"}, http.StatusInternalServerError)
		return
	}

	// Get full lobby details
	lobby, err := l.getLobbyWithMembers(lobbyID)
	if err != nil {
		log.Printf("[Lobby] Get lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to get lobby"}, http.StatusInternalServerError)
		return
	}

	l.mu.Lock()
	l.activeLobbies[lobbyID] = lobby
	l.mu.Unlock()

	log.Printf("[Lobby] Lobby created: %s by %s", lobbyID, user.Username)
	respondJSON(w, LobbyResponse{Success: true, Lobby: lobby}, http.StatusOK)
}

// Join lobby
func (l *LobbyService) handleJoinLobby(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req JoinLobbyRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Check lobby exists and has space
	lobby, err := l.db.GetLobby(req.LobbyID)
	if err != nil {
		log.Printf("[Lobby] Get lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}
	if lobby == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Lobby not found"}, http.StatusNotFound)
		return
	}

	// Check member count
	count, err := l.db.GetLobbyMemberCount(req.LobbyID)
	if err != nil {
		log.Printf("[Lobby] Member count error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}

	if count >= lobby.MaxPlayers {
		respondJSON(w, LobbyResponse{Success: false, Message: "Lobby is full"}, http.StatusConflict)
		return
	}

	// Join lobby
	if err := l.db.JoinLobby(req.LobbyID, user.ID); err != nil {
		log.Printf("[Lobby] Join error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to join lobby"}, http.StatusInternalServerError)
		return
	}

	// Get updated lobby
	lobbyWithMembers, err := l.getLobbyWithMembers(req.LobbyID)
	if err != nil {
		log.Printf("[Lobby] Get lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to get lobby"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Lobby] User %s joined lobby %s", user.Username, req.LobbyID)
	respondJSON(w, LobbyResponse{Success: true, Lobby: lobbyWithMembers}, http.StatusOK)
}

// Leave lobby
func (l *LobbyService) handleLeaveLobby(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	lobbyID := r.URL.Query().Get("lobby_id")
	if lobbyID == "" {
		respondJSON(w, LobbyResponse{Success: false, Message: "Missing lobby_id"}, http.StatusBadRequest)
		return
	}

	lobby, err := l.db.GetLobby(lobbyID)
	if err != nil || lobby == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Lobby not found"}, http.StatusNotFound)
		return
	}

	// Leave lobby
	if err := l.db.LeaveLobby(lobbyID, user.ID); err != nil {
		log.Printf("[Lobby] Leave error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to leave lobby"}, http.StatusInternalServerError)
		return
	}

	// If host left, delete lobby
	if lobby.HostID == user.ID {
		if err := l.db.DeleteLobby(lobbyID); err != nil {
			log.Printf("[Lobby] Delete lobby error: %v", err)
		}
		l.mu.Lock()
		delete(l.activeLobbies, lobbyID)
		l.mu.Unlock()
		log.Printf("[Lobby] Lobby %s deleted (host left)", lobbyID)
	}

	log.Printf("[Lobby] User %s left lobby %s", user.Username, lobbyID)
	respondJSON(w, LobbyResponse{Success: true}, http.StatusOK)
}

// Set ready status
func (l *LobbyService) handleSetReady(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	lobbyID := r.URL.Query().Get("lobby_id")
	if lobbyID == "" {
		respondJSON(w, LobbyResponse{Success: false, Message: "Missing lobby_id"}, http.StatusBadRequest)
		return
	}

	var req LobbyActionRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	if err := l.db.SetReadyStatus(lobbyID, user.ID, req.Ready); err != nil {
		log.Printf("[Lobby] Set ready error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to set ready status"}, http.StatusInternalServerError)
		return
	}

	// Get updated lobby
	lobbyWithMembers, err := l.getLobbyWithMembers(lobbyID)
	if err != nil {
		log.Printf("[Lobby] Get lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Failed to get lobby"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Lobby] User %s set ready=%v in lobby %s", user.Username, req.Ready, lobbyID)
	respondJSON(w, LobbyResponse{Success: true, Lobby: lobbyWithMembers}, http.StatusOK)
}

// Get lobby details
func (l *LobbyService) handleGetLobby(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, LobbyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	lobbyID := r.URL.Query().Get("lobby_id")
	if lobbyID == "" {
		respondJSON(w, LobbyResponse{Success: false, Message: "Missing lobby_id"}, http.StatusBadRequest)
		return
	}

	lobby, err := l.getLobbyWithMembers(lobbyID)
	if err != nil {
		log.Printf("[Lobby] Get lobby error: %v", err)
		respondJSON(w, LobbyResponse{Success: false, Message: "Lobby not found"}, http.StatusNotFound)
		return
	}

	respondJSON(w, LobbyResponse{Success: true, Lobby: lobby}, http.StatusOK)
}

// Helper to get lobby with members
func (l *LobbyService) getLobbyWithMembers(lobbyID string) (*Lobby, error) {
	lobby, err := l.db.GetLobby(lobbyID)
	if err != nil || lobby == nil {
		return nil, err
	}

	members, err := l.db.GetLobbyMembers(lobbyID)
	if err != nil {
		return nil, err
	}

	lobby.Members = members
	return lobby, nil
}

// List all active lobbies
func (l *LobbyService) handleListLobbies(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, ListLobbiesResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	lobbies, err := l.db.GetActiveLobbies()
	if err != nil {
		log.Printf("[Lobby] Get active lobbies error: %v", err)
		respondJSON(w, ListLobbiesResponse{Success: false, Message: "Failed to fetch lobbies"}, http.StatusInternalServerError)
		return
	}

	// Convert to pointers and enrich with members
	var lobbyPtrs []*Lobby
	for i := range lobbies {
		members, err := l.db.GetLobbyMembers(lobbies[i].ID)
		if err != nil {
			log.Printf("[Lobby] Get members error: %v", err)
			continue
		}
		lobbies[i].Members = members
		lobbyPtrs = append(lobbyPtrs, &lobbies[i])
	}

	respondJSON(w, ListLobbiesResponse{Success: true, Lobbies: lobbyPtrs, Count: len(lobbyPtrs)}, http.StatusOK)
}
