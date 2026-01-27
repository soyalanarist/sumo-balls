package main

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

// TestLobbyFlow tests lobby creation/join/ready workflow
func TestLobbyFlow(t *testing.T) {
	db, err := NewDatabase(":memory:")
	if err != nil {
		t.Fatalf("failed to init db: %v", err)
	}
	auth := NewAuthService(db)
	lobby := NewLobbyService(db)

	// Create two users
	user1ID, _ := db.CreateUser("alice", "hashed_pass", "alice@example.com")
	user2ID, _ := db.CreateUser("bob", "hashed_pass", "bob@example.com")

	// Create sessions for both users
	token1 := "test_token_1"
	token2 := "test_token_2"
	db.CreateSession(user1ID, token1, timeNow().Add(24*timeHour))
	db.CreateSession(user2ID, token2, timeNow().Add(24*timeHour))

	// User 1 creates a lobby
	createReqBody := CreateLobbyRequest{MaxPlayers: 4}
	createReqBodyBytes, _ := json.Marshal(createReqBody)
	createReq := httptest.NewRequest(http.MethodPost, "/lobby/create", bytes.NewReader(createReqBodyBytes))
	createReq.Header.Set("Authorization", "Bearer "+token1)
	createReq.Header.Set("Content-Type", "application/json")

	createRecorder := httptest.NewRecorder()
	wrappedCreateHandler := auth.authMiddleware(lobby.handleCreateLobby)
	wrappedCreateHandler(createRecorder, createReq)

	if createRecorder.Code != http.StatusOK {
		t.Fatalf("create lobby returned status %d", createRecorder.Code)
	}

	var createResp LobbyResponse
	json.Unmarshal(createRecorder.Body.Bytes(), &createResp)
	if !createResp.Success {
		t.Fatalf("create lobby failed: %s", createResp.Message)
	}

	lobbyID := createResp.Lobby.ID

	// User 2 joins the lobby
	joinReqBody := JoinLobbyRequest{LobbyID: lobbyID}
	joinReqBodyBytes, _ := json.Marshal(joinReqBody)
	joinReq := httptest.NewRequest(http.MethodPost, "/lobby/join", bytes.NewReader(joinReqBodyBytes))
	joinReq.Header.Set("Authorization", "Bearer "+token2)
	joinReq.Header.Set("Content-Type", "application/json")

	joinRecorder := httptest.NewRecorder()
	wrappedJoinHandler := auth.authMiddleware(lobby.handleJoinLobby)
	wrappedJoinHandler(joinRecorder, joinReq)

	if joinRecorder.Code != http.StatusOK {
		t.Fatalf("join lobby returned status %d", joinRecorder.Code)
	}

	var joinResp LobbyResponse
	json.Unmarshal(joinRecorder.Body.Bytes(), &joinResp)
	if len(joinResp.Lobby.Members) != 2 {
		t.Fatalf("expected 2 members, got %d", len(joinResp.Lobby.Members))
	}

	// User 1 sets ready
	readyReqBody := LobbyActionRequest{Ready: true}
	readyReqBodyBytes, _ := json.Marshal(readyReqBody)
	readyReq := httptest.NewRequest(http.MethodPost, "/lobby/ready?lobby_id="+lobbyID, bytes.NewReader(readyReqBodyBytes))
	readyReq.Header.Set("Authorization", "Bearer "+token1)
	readyReq.Header.Set("Content-Type", "application/json")

	readyRecorder := httptest.NewRecorder()
	wrappedReadyHandler := auth.authMiddleware(lobby.handleSetReady)
	wrappedReadyHandler(readyRecorder, readyReq)

	if readyRecorder.Code != http.StatusOK {
		t.Fatalf("set ready returned status %d", readyRecorder.Code)
	}

	var readyResp LobbyResponse
	json.Unmarshal(readyRecorder.Body.Bytes(), &readyResp)
	if !readyResp.Success {
		t.Fatalf("set ready failed: %s", readyResp.Message)
	}
}
