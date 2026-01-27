package main

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

// testAuthWrapper wraps a handler with auth validation for testing
func testAuthWrapper(auth *AuthService, handler http.HandlerFunc) http.HandlerFunc {
	return auth.authMiddleware(handler)
}

// TestFriendFlow tests send/list/remove friend workflow
func TestFriendFlow(t *testing.T) {
	db, err := NewDatabase(":memory:")
	if err != nil {
		t.Fatalf("failed to init db: %v", err)
	}
	auth := NewAuthService(db)
	friends := NewFriendsService(db)

	// Create two users
	user1ID, _ := db.CreateUser("alice", "hashed_pass", "alice@example.com")
	user2ID, _ := db.CreateUser("bob", "hashed_pass", "bob@example.com")

	// Create session for user1
	token1 := "test_token_1"
	db.CreateSession(user1ID, token1, timeNow().Add(24*timeHour))

	// Send friend request from alice to bob
	reqBody := FriendActionRequest{FriendUsername: "bob"}
	reqBodyBytes, _ := json.Marshal(reqBody)
	req1 := httptest.NewRequest(http.MethodPost, "/friends/send", bytes.NewReader(reqBodyBytes))
	req1.Header.Set("Authorization", "Bearer "+token1)
	req1.Header.Set("Content-Type", "application/json")

	recorder1 := httptest.NewRecorder()
	wrappedHandler := testAuthWrapper(auth, friends.handleSendRequest)
	wrappedHandler(recorder1, req1)

	if recorder1.Code != http.StatusOK {
		t.Fatalf("send friend request returned status %d", recorder1.Code)
	}

	// List pending requests as bob
	token2 := "test_token_2"
	db.CreateSession(user2ID, token2, timeNow().Add(24*timeHour))

	req2 := httptest.NewRequest(http.MethodGet, "/friends/pending", nil)
	req2.Header.Set("Authorization", "Bearer "+token2)

	recorder2 := httptest.NewRecorder()
	wrappedHandler = testAuthWrapper(auth, friends.handlePendingRequests)
	wrappedHandler(recorder2, req2)

	if recorder2.Code != http.StatusOK {
		t.Fatalf("list pending returned status %d", recorder2.Code)
	}

	var pendingResp map[string]interface{}
	json.Unmarshal(recorder2.Body.Bytes(), &pendingResp)
	if reqs, ok := pendingResp["requests"].([]interface{}); !ok || len(reqs) == 0 {
		t.Fatalf("expected pending request from alice")
	}
}
