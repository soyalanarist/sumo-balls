package main

import (
	"encoding/json"
	"log"
	"net/http"
)

type FriendsService struct {
	db *Database
}

func NewFriendsService(db *Database) *FriendsService {
	return &FriendsService{db: db}
}

// Send friend request
func (f *FriendsService) handleSendRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req FriendActionRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Get friend by username
	friend, err := f.db.GetUserByUsername(req.FriendUsername)
	if err != nil {
		log.Printf("[Friends] Database error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}
	if friend == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "User not found"}, http.StatusNotFound)
		return
	}

	if friend.ID == user.ID {
		respondJSON(w, FriendsResponse{Success: false, Message: "Cannot add yourself as friend"}, http.StatusBadRequest)
		return
	}

	// Send friend request
	if err := f.db.SendFriendRequest(user.ID, friend.ID); err != nil {
		log.Printf("[Friends] Send request error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Failed to send friend request"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Friends] %s sent friend request to %s", user.Username, friend.Username)
	respondJSON(w, FriendsResponse{Success: true, Message: "Friend request sent"}, http.StatusOK)
}

// Accept friend request
func (f *FriendsService) handleAcceptRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req FriendActionRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	if err := f.db.AcceptFriendRequest(user.ID, req.FriendID); err != nil {
		log.Printf("[Friends] Accept request error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Failed to accept friend request"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Friends] User %d accepted friend request from %d", user.ID, req.FriendID)
	respondJSON(w, FriendsResponse{Success: true, Message: "Friend request accepted"}, http.StatusOK)
}

// Remove friend
func (f *FriendsService) handleRemoveFriend(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodDelete {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req FriendActionRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	if err := f.db.RemoveFriend(user.ID, req.FriendID); err != nil {
		log.Printf("[Friends] Remove friend error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Failed to remove friend"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Friends] User %d removed friend %d", user.ID, req.FriendID)
	respondJSON(w, FriendsResponse{Success: true, Message: "Friend removed"}, http.StatusOK)
}

// List friends
func (f *FriendsService) handleListFriends(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	friends, err := f.db.GetFriends(user.ID)
	if err != nil {
		log.Printf("[Friends] List friends error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}

	respondJSON(w, FriendsResponse{Success: true, Friends: friends}, http.StatusOK)
}

// List pending requests
func (f *FriendsService) handlePendingRequests(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, FriendsResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	requests, err := f.db.GetPendingRequests(user.ID)
	if err != nil {
		log.Printf("[Friends] Pending requests error: %v", err)
		respondJSON(w, FriendsResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}

	respondJSON(w, FriendsResponse{Success: true, Requests: requests}, http.StatusOK)
}
