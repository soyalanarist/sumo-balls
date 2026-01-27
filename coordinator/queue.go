package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"
)

type QueueService struct {
	db             *Database
	lobbyService   *LobbyService
	mu             sync.Mutex
	activeQueues   map[string]*Queue // In-memory cache
	queueCounter   int64
	matchCounter   int64
	matchingTicker *time.Ticker // Periodically run matchmaking
}

func NewQueueService(db *Database, lobbyService *LobbyService) *QueueService {
	qs := &QueueService{
		db:           db,
		lobbyService: lobbyService,
		activeQueues: make(map[string]*Queue),
	}

	// Start background matchmaking ticker (every 2 seconds)
	qs.matchingTicker = time.NewTicker(2 * time.Second)
	go qs.backgroundMatchmaking()

	return qs
}

// Start queuing (solo or party)
func (q *QueueService) handleStartQueue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req StartQueueRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	if req.QueueType != "solo" && req.QueueType != "party" {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Invalid queue type"}, http.StatusBadRequest)
		return
	}

	// For party queue, verify user is party leader
	if req.QueueType == "party" {
		if req.PartyID == "" {
			respondJSON(w, QueueStatusResponse{Success: false, Message: "Missing party_id for party queue"}, http.StatusBadRequest)
			return
		}

		party, err := q.db.GetParty(req.PartyID)
		if err != nil || party == nil {
			respondJSON(w, QueueStatusResponse{Success: false, Message: "Party not found"}, http.StatusNotFound)
			return
		}

		if party.LeaderID != user.ID {
			respondJSON(w, QueueStatusResponse{Success: false, Message: "Only party leader can queue"}, http.StatusForbidden)
			return
		}

		// Party must have 1-6 members
		members, _ := q.db.GetPartyMembers(req.PartyID)
		if len(members) == 0 || len(members) > 6 {
			respondJSON(w, QueueStatusResponse{Success: false, Message: "Party must have 1-6 members"}, http.StatusBadRequest)
			return
		}
	}

	q.mu.Lock()
	q.queueCounter++
	queueID := fmt.Sprintf("queue_%d_%d", time.Now().Unix(), q.queueCounter)
	q.mu.Unlock()

	// Create queue in database
	if err := q.db.CreateQueue(queueID, user.ID, req.PartyID, req.QueueType); err != nil {
		log.Printf("[Queue] Create queue error: %v", err)
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Failed to start queue"}, http.StatusInternalServerError)
		return
	}

	// Get queue details
	queue, err := q.db.GetQueue(queueID)
	if err != nil {
		log.Printf("[Queue] Get queue error: %v", err)
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Failed to get queue"}, http.StatusInternalServerError)
		return
	}

	q.mu.Lock()
	q.activeQueues[queueID] = queue
	q.mu.Unlock()

	log.Printf("[Queue] %s started %s queue: %s", user.Username, req.QueueType, queueID)
	respondJSON(w, QueueStatusResponse{Success: true, Queue: queue, EstimatedWaitMs: 30000}, http.StatusOK)
}

// Cancel queue
func (q *QueueService) handleCancelQueue(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	queueID := r.URL.Query().Get("queue_id")
	if queueID == "" {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Missing queue_id"}, http.StatusBadRequest)
		return
	}

	queue, err := q.db.GetQueue(queueID)
	if err != nil || queue == nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Queue not found"}, http.StatusNotFound)
		return
	}

	// Only user who started queue can cancel it
	if queue.UserID != user.ID {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Only queue initiator can cancel"}, http.StatusForbidden)
		return
	}

	// Cancel queue
	if err := q.db.UpdateQueueState(queueID, "cancelled"); err != nil {
		log.Printf("[Queue] Cancel queue error: %v", err)
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Failed to cancel queue"}, http.StatusInternalServerError)
		return
	}

	q.mu.Lock()
	delete(q.activeQueues, queueID)
	q.mu.Unlock()

	log.Printf("[Queue] Queue cancelled: %s", queueID)
	respondJSON(w, QueueStatusResponse{Success: true}, http.StatusOK)
}

// Get queue status
func (q *QueueService) handleQueueStatus(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	queueID := r.URL.Query().Get("queue_id")
	if queueID == "" {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Missing queue_id"}, http.StatusBadRequest)
		return
	}

	queue, err := q.db.GetQueue(queueID)
	if err != nil || queue == nil {
		respondJSON(w, QueueStatusResponse{Success: false, Message: "Queue not found"}, http.StatusNotFound)
		return
	}

	respondJSON(w, QueueStatusResponse{Success: true, Queue: queue}, http.StatusOK)
}

// Accept match invitation
func (q *QueueService) handleAcceptMatch(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req AcceptMatchRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Get invitation
	invitation, err := q.db.GetMatchInvitation(req.InvitationID)
	if err != nil || invitation == nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Invitation not found"}, http.StatusNotFound)
		return
	}

	// Check if user is recipient
	if invitation.UserID != user.ID {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "This invitation is not for you"}, http.StatusForbidden)
		return
	}

	// Check if expired
	if time.Now().After(invitation.ExpiresAt) {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Invitation expired"}, http.StatusGone)
		return
	}

	// Accept invitation
	if err := q.db.UpdateMatchInvitationState(req.InvitationID, "accepted"); err != nil {
		log.Printf("[Queue] Accept match error: %v", err)
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Failed to accept match"}, http.StatusInternalServerError)
		return
	}

	// Check if all players in match have accepted
	q.checkMatchReady(invitation.QueueID, invitation.LobbyID)

	log.Printf("[Queue] User %s accepted match invitation %s", user.Username, req.InvitationID)
	respondJSON(w, MatchInvitationResponse{Success: true, Message: "Match accepted"}, http.StatusOK)
}

// Decline match invitation
func (q *QueueService) handleDeclineMatch(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	var req AcceptMatchRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Get invitation
	invitation, err := q.db.GetMatchInvitation(req.InvitationID)
	if err != nil || invitation == nil {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Invitation not found"}, http.StatusNotFound)
		return
	}

	// Check if user is recipient
	if invitation.UserID != user.ID {
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "This invitation is not for you"}, http.StatusForbidden)
		return
	}

	// Decline invitation
	if err := q.db.UpdateMatchInvitationState(req.InvitationID, "declined"); err != nil {
		log.Printf("[Queue] Decline match error: %v", err)
		respondJSON(w, MatchInvitationResponse{Success: false, Message: "Failed to decline match"}, http.StatusInternalServerError)
		return
	}

	// Decline entire queue/party
	queue, _ := q.db.GetQueue(invitation.QueueID)
	if queue != nil {
		q.db.UpdateQueueState(invitation.QueueID, "declined")
	}

	log.Printf("[Queue] User %s declined match invitation %s", user.Username, req.InvitationID)
	respondJSON(w, MatchInvitationResponse{Success: true, Message: "Match declined"}, http.StatusOK)
}

// Background matchmaking service
func (q *QueueService) backgroundMatchmaking() {
	for range q.matchingTicker.C {
		q.runMatchmaking()
	}
}

// Run matchmaking algorithm
func (q *QueueService) runMatchmaking() {
	// Get all queued players/parties
	queues, err := q.db.GetQueuesByState("queued")
	if err != nil || len(queues) == 0 {
		return
	}

	// Group by party_id (empty string = solo)
	partyGroups := make(map[string][]Queue)
	for _, queue := range queues {
		partyID := queue.PartyID
		if partyID == "" {
			// Solo queue: each gets own party_id
			partyID = fmt.Sprintf("_solo_%d", queue.UserID)
		}
		partyGroups[partyID] = append(partyGroups[partyID], queue)
	}

	// Find groups that sum to 6 players
	groups := make([][]Queue, 0)
	used := make(map[string]bool)

	// First pass: exact matches
	for partyID, queues := range partyGroups {
		if used[partyID] {
			continue
		}

		groupSize := len(queues)
		if groupSize == 6 {
			groups = append(groups, queues)
			used[partyID] = true
		} else if groupSize < 6 {
			// Try to combine with other groups
			remaining := 6 - groupSize
			currentGroup := make([]Queue, len(queues))
			copy(currentGroup, queues)

			for otherPartyID, otherQueues := range partyGroups {
				if used[otherPartyID] || otherPartyID == partyID {
					continue
				}

				otherSize := len(otherQueues)
				if otherSize <= remaining {
					currentGroup = append(currentGroup, otherQueues...)
					remaining -= otherSize
					used[otherPartyID] = true

					if remaining == 0 {
						groups = append(groups, currentGroup)
						used[partyID] = true
						break
					}
				}
			}
		}
	}

	// Create matches for found groups
	for _, group := range groups {
		q.createMatch(group)
	}
}

// Create match and lobby
func (q *QueueService) createMatch(group []Queue) {
	q.mu.Lock()
	q.matchCounter++
	lobbyID := fmt.Sprintf("lobby_match_%d_%d", time.Now().Unix(), q.matchCounter)
	q.mu.Unlock()

	// Create lobby in game server
	if err := q.db.CreateLobby(lobbyID, group[0].UserID); err != nil {
		log.Printf("[Queue] Create lobby error: %v", err)
		return
	}

	log.Printf("[Queue] Match created: %s with %d players", lobbyID, len(group))

	// Send match invitations to all players
	expiresAt := time.Now().Add(30 * time.Second)
	for _, queue := range group {
		inviteID := fmt.Sprintf("invite_%d_%d_%d", time.Now().Unix(), queue.UserID, q.matchCounter)

		if err := q.db.CreateMatchInvitation(inviteID, queue.ID, queue.UserID, lobbyID, expiresAt); err != nil {
			log.Printf("[Queue] Create invitation error: %v", err)
			continue
		}

		log.Printf("[Queue] Invitation sent: %s to user %d", inviteID, queue.UserID)
	}

	// Update queues to match_found
	for _, queue := range group {
		q.db.UpdateQueueState(queue.ID, "match_found")
	}
}

// Check if all players accepted match
func (q *QueueService) checkMatchReady(queueID, lobbyID string) {
	invitations, err := q.db.GetMatchInvitationsByQueue(queueID)
	if err != nil {
		return
	}

	allAccepted := true
	for _, inv := range invitations {
		if inv.State != "accepted" {
			allAccepted = false
			break
		}
	}

	if allAccepted {
		// All players accepted, mark queue as ready
		q.db.UpdateQueueState(queueID, "ready")
		// Lobby can start game
		q.db.UpdateLobbyState(lobbyID, "starting")
		log.Printf("[Queue] Match ready: all players accepted for lobby %s", lobbyID)
	}
}

// Stop the matchmaking service
func (q *QueueService) Stop() {
	if q.matchingTicker != nil {
		q.matchingTicker.Stop()
	}
}
