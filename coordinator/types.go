package main

import "time"

// User represents a registered user
type User struct {
	ID           int64     `json:"id"`
	Username     string    `json:"username"`
	PasswordHash string    `json:"-"` // Never send password hash to client
	Email        string    `json:"email"`
	CreatedAt    time.Time `json:"created_at"`
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
}

// FriendRequest represents a pending friend request
type FriendRequest struct {
	ID           int64     `json:"id"`
	FromUserID   int64     `json:"from_user_id"`
	FromUsername string    `json:"from_username"`
	CreatedAt    time.Time `json:"created_at"`
}

// Lobby represents a game lobby
type Lobby struct {
	ID         string    `json:"id"`
	HostID     int64     `json:"host_id"`
	State      string    `json:"state"` // 'waiting', 'starting', 'playing'
	MaxPlayers int       `json:"max_players"`
	CreatedAt  time.Time `json:"created_at"`
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
