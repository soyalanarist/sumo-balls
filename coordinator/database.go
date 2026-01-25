package main

import (
	"database/sql"
	"log"
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
		password_hash TEXT NOT NULL,
		email TEXT UNIQUE,
		created_at DATETIME DEFAULT CURRENT_TIMESTAMP
	);

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
	`

	if _, err := db.Exec(schema); err != nil {
		return nil, err
	}

	log.Println("[Database] Initialized successfully")
	return &Database{db: db}, nil
}

// User operations
func (d *Database) CreateUser(username, passwordHash, email string) (int64, error) {
	result, err := d.db.Exec(
		"INSERT INTO users (username, password_hash, email) VALUES (?, ?, ?)",
		username, passwordHash, email,
	)
	if err != nil {
		return 0, err
	}
	return result.LastInsertId()
}

func (d *Database) GetUserByUsername(username string) (*User, error) {
	var user User
	err := d.db.QueryRow(
		"SELECT id, username, password_hash, email, created_at FROM users WHERE username = ?",
		username,
	).Scan(&user.ID, &user.Username, &user.PasswordHash, &user.Email, &user.CreatedAt)
	
	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &user, nil
}

func (d *Database) GetUserByEmail(email string) (*User, error) {
	var user User
	err := d.db.QueryRow(
		"SELECT id, username, password_hash, email, created_at FROM users WHERE email = ?",
		email,
	).Scan(&user.ID, &user.Username, &user.PasswordHash, &user.Email, &user.CreatedAt)

	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
	return &user, nil
}

func (d *Database) GetUserByID(userID int64) (*User, error) {
	var user User
	err := d.db.QueryRow(
		"SELECT id, username, password_hash, email, created_at FROM users WHERE id = ?",
		userID,
	).Scan(&user.ID, &user.Username, &user.PasswordHash, &user.Email, &user.CreatedAt)
	
	if err == sql.ErrNoRows {
		return nil, nil
	}
	if err != nil {
		return nil, err
	}
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
		SELECT f.id, f.user_id, f.friend_id, f.status, u.username
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
		if err := rows.Scan(&f.ID, &f.UserID, &f.FriendID, &f.Status, &f.FriendUsername); err != nil {
			return nil, err
		}
		friends = append(friends, f)
	}
	return friends, nil
}

func (d *Database) GetPendingRequests(userID int64) ([]FriendRequest, error) {
	rows, err := d.db.Query(`
		SELECT f.id, f.user_id, u.username, f.created_at
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
		if err := rows.Scan(&r.ID, &r.FromUserID, &r.FromUsername, &r.CreatedAt); err != nil {
			return nil, err
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

func (d *Database) Close() error {
	return d.db.Close()
}
