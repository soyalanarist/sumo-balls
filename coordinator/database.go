package main

import (
	"database/sql"
	"fmt"
	"log"
	"strings"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

type Database struct {
	db *sql.DB
}

func NewDatabase(dbPath string) (*Database, error) {
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		return nil, err
	}

	// Create tables
	schema := `
	CREATE TABLE IF NOT EXISTS users (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		username TEXT UNIQUE NOT NULL,
		handle TEXT,
		tagline TEXT,
		handle_search TEXT,
		password_hash TEXT NOT NULL,
		email TEXT UNIQUE,
		handle_last_changed DATETIME,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP
	);

	CREATE UNIQUE INDEX IF NOT EXISTS idx_handle_tagline ON users(handle_search, tagline) WHERE handle_search IS NOT NULL AND tagline IS NOT NULL;

	CREATE TABLE IF NOT EXISTS friends (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		user_id INTEGER NOT NULL,
		friend_id INTEGER NOT NULL,
		status TEXT NOT NULL, -- 'pending', 'accepted'
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (user_id) REFERENCES users(id),
		FOREIGN KEY (friend_id) REFERENCES users(id),
		UNIQUE(user_id, friend_id)
	);

	CREATE TABLE IF NOT EXISTS sessions (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		user_id INTEGER NOT NULL,
		token TEXT UNIQUE NOT NULL,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		expires_at DATETIME NOT NULL,
		FOREIGN KEY (user_id) REFERENCES users(id)
	);

	CREATE TABLE IF NOT EXISTS lobbies (
		id TEXT PRIMARY KEY,
		host_id INTEGER NOT NULL,
		state TEXT NOT NULL, -- 'waiting', 'starting', 'playing'
		max_players INTEGER DEFAULT 6,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (host_id) REFERENCES users(id)
	);

	CREATE TABLE IF NOT EXISTS lobby_members (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		lobby_id TEXT NOT NULL,
		user_id INTEGER NOT NULL,
		ready_status INTEGER DEFAULT 0, -- 0 = not ready, 1 = ready
		joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (lobby_id) REFERENCES lobbies(id),
		FOREIGN KEY (user_id) REFERENCES users(id),
		UNIQUE(lobby_id, user_id)
	);

	CREATE TABLE IF NOT EXISTS parties (
		id TEXT PRIMARY KEY,
		leader_id INTEGER NOT NULL,
		state TEXT NOT NULL,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (leader_id) REFERENCES users(id)
	);

	CREATE TABLE IF NOT EXISTS party_members (
		id INTEGER PRIMARY KEY AUTOINCREMENT,
		party_id TEXT NOT NULL,
		user_id INTEGER NOT NULL,
		joined_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (party_id) REFERENCES parties(id),
		FOREIGN KEY (user_id) REFERENCES users(id),
		UNIQUE(party_id, user_id)
	);

	CREATE TABLE IF NOT EXISTS party_invites (
		id TEXT PRIMARY KEY,
		party_id TEXT NOT NULL,
		from_user_id INTEGER NOT NULL,
		to_user_id INTEGER NOT NULL,
		state TEXT NOT NULL,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		expires_at DATETIME NOT NULL,
		FOREIGN KEY (party_id) REFERENCES parties(id),
		FOREIGN KEY (from_user_id) REFERENCES users(id),
		FOREIGN KEY (to_user_id) REFERENCES users(id)
	);

	CREATE TABLE IF NOT EXISTS queues (
		id TEXT PRIMARY KEY,
		user_id INTEGER NOT NULL,
		party_id TEXT,
		queue_type TEXT NOT NULL,
		state TEXT NOT NULL,
		estimated_wait_ms INTEGER DEFAULT 0,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		FOREIGN KEY (user_id) REFERENCES users(id),
		FOREIGN KEY (party_id) REFERENCES parties(id)
	);

	CREATE TABLE IF NOT EXISTS match_invitations (
		id TEXT PRIMARY KEY,
		queue_id TEXT NOT NULL,
		user_id INTEGER NOT NULL,
		lobby_id TEXT,
		state TEXT NOT NULL,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
		expires_at DATETIME NOT NULL,
		FOREIGN KEY (queue_id) REFERENCES queues(id),
		FOREIGN KEY (user_id) REFERENCES users(id),
		FOREIGN KEY (lobby_id) REFERENCES lobbies(id)
	);
	`

	if _, err := db.Exec(schema); err != nil {
		return nil, err
	}

	log.Println("[Database] Initialized successfully")
	return &Database{db: db}, nil
}

// User operations
func (d *Database) CreateUser(username, passwordHash, email string) (int64, error) {
	return d.CreateUserWithHandle(username, "", passwordHash, email)
}

func (d *Database) CreateUserWithHandle(username, handle, passwordHash, email string) (int64, error) {
	var handleVal, handleSearchVal interface{}
	if handle != "" {
		handleVal = handle
		handleSearchVal = normalizeHandle(handle)
	} else {
		handleVal = nil
		handleSearchVal = nil
	}
	result, err := d.db.Exec(
		"INSERT INTO users (username, handle, tagline, handle_search, password_hash, email) VALUES (?, ?, ?, ?, ?, ?)",
		username, handleVal, nil, handleSearchVal, passwordHash, email,
	)
	if err != nil {
		return 0, err
	}
	return result.LastInsertId()
}

func (d *Database) GetUserByUsername(username string) (*User, error) {
	var user User
	var handle, tagline, handleSearch sql.NullString
	var handleLastChanged sql.NullTime
	err := d.db.QueryRow(
		"SELECT id, username, handle, tagline, handle_search, password_hash, email, handle_last_changed, created_at FROM users WHERE username = ?",
		username,
	).Scan(&user.ID, &user.Username, &handle, &tagline, &handleSearch, &user.PasswordHash, &user.Email, &handleLastChanged, &user.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if handle.Valid {
		user.Handle = handle.String
		user.HandleSearch = handleSearch.String
	}
	if tagline.Valid {
		user.Tagline = tagline.String
	}
	if handleLastChanged.Valid {
		user.HandleLastChanged = &handleLastChanged.Time
	}
	// OAuth users need to set handle if they don't have one
	user.NeedsHandleSetup = user.Handle == "" && user.PasswordHash == "oauth_google"
	return &user, nil
}

func (d *Database) GetUserByEmail(email string) (*User, error) {
	var user User
	var handle, tagline, handleSearch sql.NullString
	var handleLastChanged sql.NullTime
	err := d.db.QueryRow(
		"SELECT id, username, handle, tagline, handle_search, password_hash, email, handle_last_changed, created_at FROM users WHERE email = ?",
		email,
	).Scan(&user.ID, &user.Username, &handle, &tagline, &handleSearch, &user.PasswordHash, &user.Email, &handleLastChanged, &user.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if handle.Valid {
		user.Handle = handle.String
		user.HandleSearch = handleSearch.String
	}
	if tagline.Valid {
		user.Tagline = tagline.String
	}
	if handleLastChanged.Valid {
		user.HandleLastChanged = &handleLastChanged.Time
	}
	// OAuth users need to set handle if they don't have one
	user.NeedsHandleSetup = user.Handle == "" && user.PasswordHash == "oauth_google"
	return &user, nil
}

func (d *Database) GetUserByID(userID int64) (*User, error) {
	var user User
	var handle, tagline, handleSearch sql.NullString
	var handleLastChanged sql.NullTime
	err := d.db.QueryRow(
		"SELECT id, username, handle, tagline, handle_search, password_hash, email, handle_last_changed, created_at FROM users WHERE id = ?",
		userID,
	).Scan(&user.ID, &user.Username, &handle, &tagline, &handleSearch, &user.PasswordHash, &user.Email, &handleLastChanged, &user.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if handle.Valid {
		user.Handle = handle.String
		user.HandleSearch = handleSearch.String
	}
	if tagline.Valid {
		user.Tagline = tagline.String
	}
	if handleLastChanged.Valid {
		user.HandleLastChanged = &handleLastChanged.Time
	}
	// OAuth users need to set handle if they don't have one
	user.NeedsHandleSetup = user.Handle == "" && user.PasswordHash == "oauth_google"
	return &user, nil
}

// Session operations
func (d *Database) CreateSession(userID int64, token string, expiresAt time.Time) error {
	_, err := d.db.Exec(
		"INSERT INTO sessions (user_id, token, expires_at) VALUES (?, ?, ?)",
		userID, token, expiresAt,
	)
	return err
}

func (d *Database) GetSessionByToken(token string) (*Session, error) {
	var session Session
	err := d.db.QueryRow(
		"SELECT id, user_id, token, created_at, expires_at FROM sessions WHERE token = ? AND expires_at > datetime('now')",
		token,
	).Scan(&session.ID, &session.UserID, &session.Token, &session.CreatedAt, &session.ExpiresAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &session, nil
}

func (d *Database) DeleteSession(token string) error {
	_, err := d.db.Exec("DELETE FROM sessions WHERE token = ?", token)
	return err
}

// Get all active sessions for a user
func (d *Database) GetActiveSessionsByUserID(userID int64) ([]Session, error) {
	rows, err := d.db.Query(
		"SELECT id, user_id, token, created_at, expires_at FROM sessions WHERE user_id = ? AND expires_at > datetime('now')",
		userID,
	)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var sessions []Session
	for rows.Next() {
		var s Session
		if err := rows.Scan(&s.ID, &s.UserID, &s.Token, &s.CreatedAt, &s.ExpiresAt); err != nil {
			return nil, err
		}
		sessions = append(sessions, s)
	}
	return sessions, nil
}

// Delete all sessions for a user (for logout/force logout)
func (d *Database) DeleteAllUserSessions(userID int64) error {
	_, err := d.db.Exec("DELETE FROM sessions WHERE user_id = ?", userID)
	return err
}

// Friend operations
func (d *Database) SendFriendRequest(userID, friendID int64) error {
	_, err := d.db.Exec(
		"INSERT INTO friends (user_id, friend_id, status) VALUES (?, ?, 'pending')",
		userID, friendID,
	)
	return err
}

func (d *Database) AcceptFriendRequest(userID, friendID int64) error {
	_, err := d.db.Exec(
		"UPDATE friends SET status = 'accepted' WHERE user_id = ? AND friend_id = ? AND status = 'pending'",
		friendID, userID,
	)
	return err
}

func (d *Database) RemoveFriend(userID, friendID int64) error {
	_, err := d.db.Exec(
		"DELETE FROM friends WHERE (user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)",
		userID, friendID, friendID, userID,
	)
	return err
}

func (d *Database) GetFriends(userID int64) ([]Friend, error) {
	rows, err := d.db.Query(`
		SELECT f.id, f.user_id, f.friend_id, f.status, u.username, u.handle, u.tagline
		FROM friends f
		JOIN users u ON (f.friend_id = u.id AND f.user_id = ?)
		   OR (f.user_id = u.id AND f.friend_id = ?)
		WHERE (f.user_id = ? OR f.friend_id = ?) AND f.status = 'accepted'
	`, userID, userID, userID, userID)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var friends []Friend
	for rows.Next() {
		var f Friend
		var handle, tagline sql.NullString
		if err := rows.Scan(&f.ID, &f.UserID, &f.FriendID, &f.Status, &f.FriendUsername, &handle, &tagline); err != nil {
			return nil, err
		}
		if handle.Valid {
			f.FriendHandle = handle.String
		}
		if tagline.Valid {
			f.FriendTagline = tagline.String
		}
		friends = append(friends, f)
	}
	return friends, nil
}

func (d *Database) GetPendingRequests(userID int64) ([]FriendRequest, error) {
	rows, err := d.db.Query(`
		SELECT f.id, f.user_id, u.username, u.handle, u.tagline, f.created_at
		FROM friends f
		JOIN users u ON f.user_id = u.id
		WHERE f.friend_id = ? AND f.status = 'pending'
	`, userID)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var requests []FriendRequest
	for rows.Next() {
		var r FriendRequest
		var handle, tagline sql.NullString
		if err := rows.Scan(&r.ID, &r.FromUserID, &r.FromUsername, &handle, &tagline, &r.CreatedAt); err != nil {
			return nil, err
		}
		if handle.Valid {
			r.FromHandle = handle.String
		}
		if tagline.Valid {
			r.FromTagline = tagline.String
		}
		requests = append(requests, r)
	}
	return requests, nil
}

// Lobby operations
func (d *Database) CreateLobby(lobbyID string, hostID int64) error {
	_, err := d.db.Exec(
		"INSERT INTO lobbies (id, host_id, state) VALUES (?, ?, 'waiting')",
		lobbyID, hostID,
	)
	return err
}

func (d *Database) GetLobby(lobbyID string) (*Lobby, error) {
	var lobby Lobby
	err := d.db.QueryRow(
		"SELECT id, host_id, state, max_players, created_at FROM lobbies WHERE id = ?",
		lobbyID,
	).Scan(&lobby.ID, &lobby.HostID, &lobby.State, &lobby.MaxPlayers, &lobby.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &lobby, nil
}

func (d *Database) UpdateLobbyState(lobbyID, state string) error {
	_, err := d.db.Exec("UPDATE lobbies SET state = ? WHERE id = ?", state, lobbyID)
	return err
}

func (d *Database) DeleteLobby(lobbyID string) error {
	_, err := d.db.Exec("DELETE FROM lobbies WHERE id = ?", lobbyID)
	return err
}

func (d *Database) JoinLobby(lobbyID string, userID int64) error {
	_, err := d.db.Exec(
		"INSERT INTO lobby_members (lobby_id, user_id, ready_status) VALUES (?, ?, 0)",
		lobbyID, userID,
	)
	return err
}

func (d *Database) LeaveLobby(lobbyID string, userID int64) error {
	_, err := d.db.Exec(
		"DELETE FROM lobby_members WHERE lobby_id = ? AND user_id = ?",
		lobbyID, userID,
	)
	return err
}

func (d *Database) SetReadyStatus(lobbyID string, userID int64, ready bool) error {
	readyInt := 0
	if ready {
		readyInt = 1
	}
	_, err := d.db.Exec(
		"UPDATE lobby_members SET ready_status = ? WHERE lobby_id = ? AND user_id = ?",
		readyInt, lobbyID, userID,
	)
	return err
}

func (d *Database) GetLobbyMembers(lobbyID string) ([]LobbyMember, error) {
	rows, err := d.db.Query(`
		SELECT lm.user_id, u.username, lm.ready_status
		FROM lobby_members lm
		JOIN users u ON lm.user_id = u.id
		WHERE lm.lobby_id = ?
	`, lobbyID)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var members []LobbyMember
	for rows.Next() {
		var m LobbyMember
		if err := rows.Scan(&m.UserID, &m.Username, &m.ReadyStatus); err != nil {
			return nil, err
		}
		members = append(members, m)
	}
	return members, nil
}

func (d *Database) GetLobbyMemberCount(lobbyID string) (int, error) {
	var count int
	err := d.db.QueryRow("SELECT COUNT(*) FROM lobby_members WHERE lobby_id = ?", lobbyID).Scan(&count)
	return count, err
}

func (d *Database) GetActiveLobbies() ([]Lobby, error) {
	rows, err := d.db.Query(`
		SELECT id, host_id, max_players, current_state, created_at
		FROM lobbies
		WHERE current_state IN ('open', 'in_progress')
		ORDER BY created_at DESC
	`)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var lobbies []Lobby
	for rows.Next() {
		var l Lobby
		if err := rows.Scan(&l.ID, &l.HostID, &l.MaxPlayers, &l.State, &l.CreatedAt); err != nil {
			return nil, err
		}
		lobbies = append(lobbies, l)
	}
	return lobbies, nil
}

// Handle operations
func (d *Database) GetUserByHandle(handle string) (*User, error) {
	handleSearch := normalizeHandle(handle)
	var user User
	var handleVal, tagline, handleSearchVal sql.NullString
	var handleLastChanged sql.NullTime
	err := d.db.QueryRow(
		"SELECT id, username, handle, tagline, handle_search, password_hash, email, handle_last_changed, created_at FROM users WHERE handle_search = ?",
		handleSearch,
	).Scan(&user.ID, &user.Username, &handleVal, &tagline, &handleSearchVal, &user.PasswordHash, &user.Email, &handleLastChanged, &user.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if handleVal.Valid {
		user.Handle = handleVal.String
		user.HandleSearch = handleSearchVal.String
	}
	if tagline.Valid {
		user.Tagline = tagline.String
	}
	if handleLastChanged.Valid {
		user.HandleLastChanged = &handleLastChanged.Time
	}
	user.NeedsHandleSetup = user.Handle == "" && user.PasswordHash == "oauth_google"
	return &user, nil
}

func (d *Database) IsHandleAvailable(handle string) (bool, error) {
	handleSearch := normalizeHandle(handle)
	var count int
	err := d.db.QueryRow("SELECT COUNT(*) FROM users WHERE handle_search = ?", handleSearch).Scan(&count)
	if err != nil {
		return false, err
	}
	return count == 0, nil
}

// Check if handle+tagline combination is available
func (d *Database) IsHandleTaglineAvailable(handle, tagline string) (bool, error) {
	handleSearch := normalizeHandle(handle)
	var count int
	err := d.db.QueryRow(
		"SELECT COUNT(*) FROM users WHERE handle_search = ? AND tagline = ?",
		handleSearch, tagline,
	).Scan(&count)
	if err != nil {
		return false, err
	}
	return count == 0, nil
}

func (d *Database) SetUserHandle(userID int64, handle, tagline string) error {
	handleSearch := normalizeHandle(handle)
	_, err := d.db.Exec(
		"UPDATE users SET handle = ?, tagline = ?, handle_search = ?, handle_last_changed = ? WHERE id = ?",
		handle, tagline, handleSearch, time.Now(), userID,
	)
	return err
}

// Party operations
func (d *Database) CreateParty(partyID string, leaderID int64) error {
	_, err := d.db.Exec(
		"INSERT INTO parties (id, leader_id, state) VALUES (?, ?, ?)",
		partyID, leaderID, "forming",
	)
	return err
}

func (d *Database) GetParty(partyID string) (*Party, error) {
	var party Party
	err := d.db.QueryRow(
		"SELECT id, leader_id, state, created_at, updated_at FROM parties WHERE id = ?",
		partyID,
	).Scan(&party.ID, &party.LeaderID, &party.State, &party.CreatedAt, &party.UpdatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &party, nil
}

func (d *Database) DeleteParty(partyID string) error {
	_, err := d.db.Exec("DELETE FROM parties WHERE id = ?", partyID)
	return err
}

func (d *Database) AddPartyMember(partyID string, userID int64) error {
	_, err := d.db.Exec(
		"INSERT INTO party_members (party_id, user_id) VALUES (?, ?)",
		partyID, userID,
	)
	return err
}

func (d *Database) RemovePartyMember(partyID string, userID int64) error {
	_, err := d.db.Exec("DELETE FROM party_members WHERE party_id = ? AND user_id = ?", partyID, userID)
	return err
}

func (d *Database) GetPartyMembers(partyID string) ([]PartyMember, error) {
	rows, err := d.db.Query(`
		SELECT pm.user_id, u.username, u.handle, p.leader_id = pm.user_id, pm.joined_at
		FROM party_members pm
		JOIN users u ON pm.user_id = u.id
		JOIN parties p ON pm.party_id = p.id
		WHERE pm.party_id = ?
		ORDER BY pm.joined_at ASC
	`, partyID)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var members []PartyMember
	for rows.Next() {
		var m PartyMember
		var handle sql.NullString
		if err := rows.Scan(&m.UserID, &m.Username, &handle, &m.IsLeader, &m.JoinedAt); err != nil {
			return nil, err
		}
		if handle.Valid {
			m.Handle = handle.String
		}
		members = append(members, m)
	}
	return members, rows.Err()
}

func (d *Database) IsPartyMember(partyID string, userID int64) (bool, error) {
	var count int
	err := d.db.QueryRow(
		"SELECT COUNT(*) FROM party_members WHERE party_id = ? AND user_id = ?",
		partyID, userID,
	).Scan(&count)
	return count > 0, err
}

func (d *Database) SendPartyInvite(partyID string, fromUserID, toUserID int64) error {
	inviteID := fmt.Sprintf("pinv_%d_%d", time.Now().Unix(), toUserID)
	expiresAt := time.Now().Add(24 * time.Hour)

	_, err := d.db.Exec(
		"INSERT INTO party_invites (id, party_id, from_user_id, to_user_id, state, expires_at) VALUES (?, ?, ?, ?, ?, ?)",
		inviteID, partyID, fromUserID, toUserID, "pending", expiresAt,
	)
	return err
}

func (d *Database) GetPartyInvite(inviteID string) (*PartyInvite, error) {
	var invite PartyInvite
	err := d.db.QueryRow(
		"SELECT id, party_id, from_user_id, to_user_id, state, created_at, expires_at FROM party_invites WHERE id = ?",
		inviteID,
	).Scan(&invite.ID, &invite.PartyID, &invite.FromUserID, &invite.ToUserID, &invite.State, &invite.CreatedAt, &invite.ExpiresAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &invite, nil
}

func (d *Database) AcceptPartyInvite(inviteID string) error {
	_, err := d.db.Exec("UPDATE party_invites SET state = ? WHERE id = ?", "accepted", inviteID)
	return err
}

func (d *Database) TransferPartyLeadership(partyID string, newLeaderID int64) error {
	_, err := d.db.Exec("UPDATE parties SET leader_id = ? WHERE id = ?", newLeaderID, partyID)
	return err
}

// Queue operations
func (d *Database) CreateQueue(queueID string, userID int64, partyID, queueType string) error {
	_, err := d.db.Exec(
		"INSERT INTO queues (id, user_id, party_id, queue_type, state) VALUES (?, ?, ?, ?, ?)",
		queueID, userID, partyID, queueType, "queued",
	)
	return err
}

func (d *Database) GetQueue(queueID string) (*Queue, error) {
	var queue Queue
	var partyID sql.NullString
	err := d.db.QueryRow(
		"SELECT id, user_id, party_id, queue_type, state, estimated_wait_ms, created_at, updated_at FROM queues WHERE id = ?",
		queueID,
	).Scan(&queue.ID, &queue.UserID, &partyID, &queue.QueueType, &queue.State, &queue.EstimatedWaitMs, &queue.CreatedAt, &queue.UpdatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if partyID.Valid {
		queue.PartyID = partyID.String
	}
	return &queue, nil
}

func (d *Database) GetQueuesByState(state string) ([]Queue, error) {
	rows, err := d.db.Query(
		"SELECT id, user_id, party_id, queue_type, state, estimated_wait_ms, created_at, updated_at FROM queues WHERE state = ?",
		state,
	)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var queues []Queue
	for rows.Next() {
		var q Queue
		var partyID sql.NullString
		if err := rows.Scan(&q.ID, &q.UserID, &partyID, &q.QueueType, &q.State, &q.EstimatedWaitMs, &q.CreatedAt, &q.UpdatedAt); err != nil {
			return nil, err
		}
		if partyID.Valid {
			q.PartyID = partyID.String
		}
		queues = append(queues, q)
	}
	return queues, rows.Err()
}

func (d *Database) UpdateQueueState(queueID, state string) error {
	_, err := d.db.Exec(
		"UPDATE queues SET state = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?",
		state, queueID,
	)
	return err
}

// Match invitation operations
func (d *Database) CreateMatchInvitation(inviteID, queueID string, userID int64, lobbyID string, expiresAt time.Time) error {
	_, err := d.db.Exec(
		"INSERT INTO match_invitations (id, queue_id, user_id, lobby_id, state, expires_at) VALUES (?, ?, ?, ?, ?, ?)",
		inviteID, queueID, userID, lobbyID, "pending", expiresAt,
	)
	return err
}

func (d *Database) GetMatchInvitation(inviteID string) (*MatchInvitation, error) {
	var invitation MatchInvitation
	var lobbyID sql.NullString
	err := d.db.QueryRow(
		"SELECT id, queue_id, user_id, lobby_id, state, created_at, expires_at FROM match_invitations WHERE id = ?",
		inviteID,
	).Scan(&invitation.ID, &invitation.QueueID, &invitation.UserID, &lobbyID, &invitation.State, &invitation.CreatedAt, &invitation.ExpiresAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	if lobbyID.Valid {
		invitation.LobbyID = lobbyID.String
	}
	return &invitation, nil
}

func (d *Database) GetMatchInvitationsByQueue(queueID string) ([]MatchInvitation, error) {
	rows, err := d.db.Query(
		"SELECT id, queue_id, user_id, lobby_id, state, created_at, expires_at FROM match_invitations WHERE queue_id = ?",
		queueID,
	)

	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var invitations []MatchInvitation
	for rows.Next() {
		var inv MatchInvitation
		var lobbyID sql.NullString
		if err := rows.Scan(&inv.ID, &inv.QueueID, &inv.UserID, &lobbyID, &inv.State, &inv.CreatedAt, &inv.ExpiresAt); err != nil {
			return nil, err
		}
		if lobbyID.Valid {
			inv.LobbyID = lobbyID.String
		}
		invitations = append(invitations, inv)
	}
	return invitations, rows.Err()
}

func (d *Database) UpdateMatchInvitationState(inviteID, state string) error {
	_, err := d.db.Exec("UPDATE match_invitations SET state = ? WHERE id = ?", state, inviteID)
	return err
}

func (d *Database) Close() error {
	return d.db.Close()
}

// Helper functions
func normalizeHandle(handle string) string {
	// Lowercase and trim for case-insensitive comparison
	return strings.ToLower(strings.TrimSpace(handle))
}
