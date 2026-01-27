package main

import "time"

// User represents a registered user
type User struct {
	ID                int64      `json:"id"`
	Username          string     `json:"username"`          // Internal username (may be auto-generated for OAuth)
	Handle            string     `json:"handle,omitempty"`  // User-chosen display name/handle
	Tagline           string     `json:"tagline,omitempty"` // 4-char unique identifier (like Discord discriminator)
	HandleSearch      string     `json:"-"`                 // Lowercase normalized handle for lookups
	PasswordHash      string     `json:"-"`                 // Never send password hash to client
	Email             string     `json:"email"`
	NeedsHandleSetup  bool       `json:"needs_handle_setup"` // True if OAuth user hasn't chosen handle yet
	HandleLastChanged *time.Time `json:"-"`                  // Last time handle was changed (for rate limiting)
	CreatedAt         time.Time  `json:"created_at"`
}

// Session represents an authenticated session
type Session struct {
	ID        int64     `json:"id"`
	UserID    int64     `json:"user_id"`
	Token     string    `json:"token"`
	CreatedAt time.Time `json:"created_at"`
	ExpiresAt time.Time `json:"expires_at"`
}

// Friend represents a friendship relationship
type Friend struct {
	ID             int64  `json:"id"`
	UserID         int64  `json:"user_id"`
	FriendID       int64  `json:"friend_id"`
	Status         string `json:"status"`
	FriendUsername string `json:"friend_username"`
	FriendHandle   string `json:"friend_handle,omitempty"`
	FriendTagline  string `json:"friend_tagline,omitempty"`
}

// FriendRequest represents a pending friend request
type FriendRequest struct {
	ID           int64     `json:"id"`
	FromUserID   int64     `json:"from_user_id"`
	FromUsername string    `json:"from_username"`
	FromHandle   string    `json:"from_handle,omitempty"`
	FromTagline  string    `json:"from_tagline,omitempty"`
	CreatedAt    time.Time `json:"created_at"`
}

// Lobby represents a game lobby
type Lobby struct {
	ID         string        `json:"id"`
	HostID     int64         `json:"host_id"`
	State      string        `json:"state"` // 'waiting', 'starting', 'playing'
	MaxPlayers int           `json:"max_players"`
	CreatedAt  time.Time     `json:"created_at"`
	Members    []LobbyMember `json:"members,omitempty"`
}

// LobbyMember represents a player in a lobby
type LobbyMember struct {
	UserID      int64  `json:"user_id"`
	Username    string `json:"username"`
	ReadyStatus int    `json:"ready_status"` // 0 = not ready, 1 = ready
}

// Auth request/response types
type RegisterRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
	Email    string `json:"email"`
}

type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

type AuthResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message,omitempty"`
	Token   string `json:"token,omitempty"`
	User    *User  `json:"user,omitempty"`
}

// Friend request/response types
type FriendActionRequest struct {
	FriendUsername string `json:"friend_username,omitempty"`
	FriendID       int64  `json:"friend_id,omitempty"`
}

type FriendsResponse struct {
	Success  bool            `json:"success"`
	Friends  []Friend        `json:"friends,omitempty"`
	Requests []FriendRequest `json:"requests,omitempty"`
	Message  string          `json:"message,omitempty"`
}

// Username setup types
type HandleAvailabilityRequest struct {
	Handle string `json:"handle"`
}

type HandleAvailabilityResponse struct {
	Success   bool   `json:"success"`
	Available bool   `json:"available"`
	Message   string `json:"message,omitempty"`
}

type SetHandleRequest struct {
	Handle  string `json:"handle"`
	Tagline string `json:"tagline"`
}

type SetHandleResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message,omitempty"`
	User    *User  `json:"user,omitempty"`
}

// Lobby request/response types
type CreateLobbyRequest struct {
	MaxPlayers int `json:"max_players,omitempty"` // Default 6
}

type JoinLobbyRequest struct {
	LobbyID string `json:"lobby_id"`
}

type LobbyActionRequest struct {
	Ready bool `json:"ready,omitempty"`
}

type LobbyResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message,omitempty"`
	Lobby   *Lobby `json:"lobby,omitempty"`
}
type ListLobbiesResponse struct {
	Success bool     `json:"success"`
	Message string   `json:"message,omitempty"`
	Lobbies []*Lobby `json:"lobbies,omitempty"`
	Count   int      `json:"count"`
}

// Party types
type Party struct {
	ID        string        `json:"id"`
	LeaderID  int64         `json:"leader_id"`
	State     string        `json:"state"` // 'forming', 'queued', 'in_match', 'disbanded'
	CreatedAt time.Time     `json:"created_at"`
	UpdatedAt time.Time     `json:"updated_at"`
	Members   []PartyMember `json:"members,omitempty"`
}

type PartyMember struct {
	UserID   int64     `json:"user_id"`
	Username string    `json:"username"`
	Handle   string    `json:"handle,omitempty"`
	IsLeader bool      `json:"is_leader"`
	JoinedAt time.Time `json:"joined_at"`
}

type PartyInvite struct {
	ID         string    `json:"id"`
	PartyID    string    `json:"party_id"`
	FromUserID int64     `json:"from_user_id"`
	FromHandle string    `json:"from_handle,omitempty"`
	ToUserID   int64     `json:"to_user_id"`
	State      string    `json:"state"` // 'pending', 'accepted', 'declined', 'expired'
	CreatedAt  time.Time `json:"created_at"`
	ExpiresAt  time.Time `json:"expires_at"`
}

// Queue types
type Queue struct {
	ID              string    `json:"id"`
	UserID          int64     `json:"user_id"`
	PartyID         string    `json:"party_id,omitempty"`
	QueueType       string    `json:"queue_type"` // 'solo', 'party'
	State           string    `json:"state"`      // 'queued', 'match_found', 'accepted', 'declined', 'ready', 'cancelled'
	EstimatedWaitMs int       `json:"estimated_wait_ms"`
	CreatedAt       time.Time `json:"created_at"`
	UpdatedAt       time.Time `json:"updated_at"`
}

type MatchInvitation struct {
	ID        string    `json:"id"`
	QueueID   string    `json:"queue_id"`
	UserID    int64     `json:"user_id"`
	LobbyID   string    `json:"lobby_id,omitempty"`
	State     string    `json:"state"` // 'pending', 'accepted', 'declined', 'expired'
	CreatedAt time.Time `json:"created_at"`
	ExpiresAt time.Time `json:"expires_at"`
}

// Party request/response types
type CreatePartyRequest struct {
	FriendUsernames []string `json:"friend_usernames,omitempty"` // Who to invite
}

type PartyResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message,omitempty"`
	Party   *Party `json:"party,omitempty"`
}

type PartyInviteRequest struct {
	FriendUsername string `json:"friend_username"`
}

type PartyInviteResponse struct {
	Success bool          `json:"success"`
	Message string        `json:"message,omitempty"`
	Invites []PartyInvite `json:"invites,omitempty"`
}

type AcceptPartyInviteRequest struct {
	InviteID string `json:"invite_id"`
}

// Queue request/response types
type StartQueueRequest struct {
	QueueType string `json:"queue_type"` // 'solo' or 'party'
	PartyID   string `json:"party_id,omitempty"`
}

type QueueStatusResponse struct {
	Success         bool   `json:"success"`
	Message         string `json:"message,omitempty"`
	Queue           *Queue `json:"queue,omitempty"`
	EstimatedWaitMs int    `json:"estimated_wait_ms,omitempty"`
}

type MatchInvitationResponse struct {
	Success     bool             `json:"success"`
	Message     string           `json:"message,omitempty"`
	Invitation  *MatchInvitation `json:"invitation,omitempty"`
	SecondsLeft int              `json:"seconds_left,omitempty"`
}

type AcceptMatchRequest struct {
	InvitationID string `json:"invitation_id"`
}
