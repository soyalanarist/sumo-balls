package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"
)

type PartyService struct {
	db            *Database
	mu            sync.Mutex
	activeParties map[string]*Party // In-memory cache
	partyCounter  int64
}

func NewPartyService(db *Database) *PartyService {
	return &PartyService{
		db:            db,
		activeParties: make(map[string]*Party),
	}
}

// Create party (user initiates)
func (p *PartyService) handleCreateParty(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req CreatePartyRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		// User might not send any invites initially
		req.FriendUsernames = []string{}
	}

	p.mu.Lock()
	p.partyCounter++
	partyID := fmt.Sprintf("party_%d_%d", time.Now().Unix(), p.partyCounter)
	p.mu.Unlock()

	// Create party in database
	if err := p.db.CreateParty(partyID, user.ID); err != nil {
		log.Printf("[Party] Create party error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to create party"}, http.StatusInternalServerError)
		return
	}

	// Add creator as member
	if err := p.db.AddPartyMember(partyID, user.ID); err != nil {
		log.Printf("[Party] Add member error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to add party member"}, http.StatusInternalServerError)
		return
	}

	// Invite friends if provided
	for _, friendUsername := range req.FriendUsernames {
		friend, err := p.db.GetUserByUsername(friendUsername)
		if err != nil || friend == nil {
			log.Printf("[Party] Friend not found: %s", friendUsername)
			continue
		}

		if err := p.db.SendPartyInvite(partyID, user.ID, friend.ID); err != nil {
			log.Printf("[Party] Send invite error: %v", err)
		}
	}

	// Get full party details
	party, err := p.getPartyWithMembers(partyID)
	if err != nil {
		log.Printf("[Party] Get party error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to get party"}, http.StatusInternalServerError)
		return
	}

	p.mu.Lock()
	p.activeParties[partyID] = party
	p.mu.Unlock()

	log.Printf("[Party] Party created: %s by %s", partyID, user.Username)
	respondJSON(w, PartyResponse{Success: true, Party: party}, http.StatusOK)
}

// Get party details
func (p *PartyService) handleGetParty(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	partyID := r.URL.Query().Get("party_id")
	if partyID == "" {
		respondJSON(w, PartyResponse{Success: false, Message: "Missing party_id"}, http.StatusBadRequest)
		return
	}

	party, err := p.getPartyWithMembers(partyID)
	if err != nil {
		log.Printf("[Party] Get party error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Party not found"}, http.StatusNotFound)
		return
	}

	respondJSON(w, PartyResponse{Success: true, Party: party}, http.StatusOK)
}

// Invite friend to party
func (p *PartyService) handleInviteFriend(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	partyID := r.URL.Query().Get("party_id")
	if partyID == "" {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Missing party_id"}, http.StatusBadRequest)
		return
	}

	var req PartyInviteRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Check user is party leader
	party, err := p.db.GetParty(partyID)
	if err != nil || party == nil {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Party not found"}, http.StatusNotFound)
		return
	}

	if party.LeaderID != user.ID {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Only party leader can invite"}, http.StatusForbidden)
		return
	}

	// Find friend
	friend, err := p.db.GetUserByUsername(req.FriendUsername)
	if err != nil || friend == nil {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Friend not found"}, http.StatusNotFound)
		return
	}

	if friend.ID == user.ID {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Cannot invite yourself"}, http.StatusBadRequest)
		return
	}

	// Check if already in party
	isMember, _ := p.db.IsPartyMember(partyID, friend.ID)
	if isMember {
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Already in party"}, http.StatusConflict)
		return
	}

	// Send invite
	if err := p.db.SendPartyInvite(partyID, user.ID, friend.ID); err != nil {
		log.Printf("[Party] Send invite error: %v", err)
		respondJSON(w, PartyInviteResponse{Success: false, Message: "Failed to send invite or invite already exists"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Party] %s invited %s to party %s", user.Username, friend.Username, partyID)
	respondJSON(w, PartyInviteResponse{Success: true, Message: "Invite sent"}, http.StatusOK)
}

// Accept party invite
func (p *PartyService) handleAcceptInvite(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req AcceptPartyInviteRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Get invite
	invite, err := p.db.GetPartyInvite(req.InviteID)
	if err != nil || invite == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Invite not found"}, http.StatusNotFound)
		return
	}

	// Check user is recipient
	if invite.ToUserID != user.ID {
		respondJSON(w, PartyResponse{Success: false, Message: "This invite is not for you"}, http.StatusForbidden)
		return
	}

	// Check invite not expired
	if time.Now().After(invite.ExpiresAt) {
		respondJSON(w, PartyResponse{Success: false, Message: "Invite expired"}, http.StatusGone)
		return
	}

	// Accept and add to party
	if err := p.db.AcceptPartyInvite(req.InviteID); err != nil {
		log.Printf("[Party] Accept invite error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to accept invite"}, http.StatusInternalServerError)
		return
	}

	if err := p.db.AddPartyMember(invite.PartyID, user.ID); err != nil {
		log.Printf("[Party] Add member error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to join party"}, http.StatusInternalServerError)
		return
	}

	// Get updated party
	party, err := p.getPartyWithMembers(invite.PartyID)
	if err != nil {
		log.Printf("[Party] Get party error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to get party"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Party] %s accepted invite to party %s", user.Username, invite.PartyID)
	respondJSON(w, PartyResponse{Success: true, Party: party}, http.StatusOK)
}

// Leave party
func (p *PartyService) handleLeaveParty(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	partyID := r.URL.Query().Get("party_id")
	if partyID == "" {
		respondJSON(w, PartyResponse{Success: false, Message: "Missing party_id"}, http.StatusBadRequest)
		return
	}

	party, err := p.db.GetParty(partyID)
	if err != nil || party == nil {
		respondJSON(w, PartyResponse{Success: false, Message: "Party not found"}, http.StatusNotFound)
		return
	}

	// Remove user from party
	if err := p.db.RemovePartyMember(partyID, user.ID); err != nil {
		log.Printf("[Party] Remove member error: %v", err)
		respondJSON(w, PartyResponse{Success: false, Message: "Failed to leave party"}, http.StatusInternalServerError)
		return
	}

	// If leader left, transfer leadership
	if party.LeaderID == user.ID {
		members, _ := p.db.GetPartyMembers(partyID)
		if len(members) > 0 {
			// Transfer to oldest member
			if err := p.db.TransferPartyLeadership(partyID, members[0].UserID); err != nil {
				log.Printf("[Party] Transfer leadership error: %v", err)
			}
		} else {
			// Disband empty party
			p.db.DeleteParty(partyID)
		}
	}

	p.mu.Lock()
	delete(p.activeParties, partyID)
	p.mu.Unlock()

	log.Printf("[Party] User %s left party %s", user.Username, partyID)
	respondJSON(w, PartyResponse{Success: true}, http.StatusOK)
}

// Helper to get party with members
func (p *PartyService) getPartyWithMembers(partyID string) (*Party, error) {
	party, err := p.db.GetParty(partyID)
	if err != nil || party == nil {
		return nil, err
	}

	members, err := p.db.GetPartyMembers(partyID)
	if err != nil {
		return nil, err
	}

	party.Members = members
	return party, nil
}
