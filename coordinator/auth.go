package main

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"os"
	"strings"
	"sync"

	"golang.org/x/oauth2"
	"golang.org/x/oauth2/google"

	"golang.org/x/crypto/bcrypt"
)

type AuthService struct {
	db *Database
	// Google OAuth support
	oauthConfig       *oauth2.Config
	pendingStates     map[string]string // state -> sessionId
	completedSessions map[string]string // sessionId -> issued auth token
	mu                sync.Mutex
}

func NewAuthService(db *Database) *AuthService {
	// Initialize OAuth config from environment variables
	clientID := getenvDefault("GOOGLE_CLIENT_ID", "")
	clientSecret := getenvDefault("GOOGLE_CLIENT_SECRET", "")
	var cfg *oauth2.Config
	if clientID != "" && clientSecret != "" {
		cfg = &oauth2.Config{
			ClientID:     clientID,
			ClientSecret: clientSecret,
			RedirectURL:  "http://localhost:8888/auth/google/callback",
			Scopes:       []string{"openid", "email", "profile"},
			Endpoint:     google.Endpoint,
		}
		log.Println("[Auth] Google OAuth configured")
	} else {
		log.Println("[Auth] Google OAuth not configured (set GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET)")
	}

	return &AuthService{
		db:                db,
		oauthConfig:       cfg,
		pendingStates:     make(map[string]string),
		completedSessions: make(map[string]string),
	}
}

// Register handles user registration
func (a *AuthService) handleRegister(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req RegisterRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Validate input
	if len(req.Username) < 3 || len(req.Username) > 20 {
		respondJSON(w, AuthResponse{Success: false, Message: "Username must be 3-20 characters"}, http.StatusBadRequest)
		return
	}
	if len(req.Password) < 6 {
		respondJSON(w, AuthResponse{Success: false, Message: "Password must be at least 6 characters"}, http.StatusBadRequest)
		return
	}

	// Check if user exists
	existingUser, err := a.db.GetUserByUsername(req.Username)
	if err != nil {
		log.Printf("[Auth] Database error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}
	if existingUser != nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Username already taken"}, http.StatusConflict)
		return
	}

	// Hash password
	passwordHash, err := bcrypt.GenerateFromPassword([]byte(req.Password), bcrypt.DefaultCost)
	if err != nil {
		log.Printf("[Auth] Password hashing error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}

	// Create user
	userID, err := a.db.CreateUser(req.Username, string(passwordHash), req.Email)
	if err != nil {
		log.Printf("[Auth] User creation error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Failed to create user"}, http.StatusInternalServerError)
		return
	}

	// Create session
	token := generateSecureToken()
	expiresAt := timeNow().Add(24 * 7 * timeHour) // 7 days
	if err := a.db.CreateSession(userID, token, expiresAt); err != nil {
		log.Printf("[Auth] Session creation error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Failed to create session"}, http.StatusInternalServerError)
		return
	}

	// Get user for response
	user, _ := a.db.GetUserByID(userID)

	log.Printf("[Auth] User registered: %s (ID: %d)", req.Username, userID)
	respondJSON(w, AuthResponse{
		Success: true,
		Token:   token,
		User:    user,
	}, http.StatusOK)
}

// Login handles user authentication
func (a *AuthService) handleLogin(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req LoginRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Invalid request"}, http.StatusBadRequest)
		return
	}

	// Get user
	user, err := a.db.GetUserByUsername(req.Username)
	if err != nil {
		log.Printf("[Auth] Database error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Server error"}, http.StatusInternalServerError)
		return
	}
	if user == nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Invalid username or password"}, http.StatusUnauthorized)
		return
	}

	// Verify password
	if err := bcrypt.CompareHashAndPassword([]byte(user.PasswordHash), []byte(req.Password)); err != nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Invalid username or password"}, http.StatusUnauthorized)
		return
	}

	// Create session
	token := generateSecureToken()
	expiresAt := timeNow().Add(24 * 7 * timeHour) // 7 days
	if err := a.db.CreateSession(user.ID, token, expiresAt); err != nil {
		log.Printf("[Auth] Session creation error: %v", err)
		respondJSON(w, AuthResponse{Success: false, Message: "Failed to create session"}, http.StatusInternalServerError)
		return
	}

	log.Printf("[Auth] User logged in: %s", user.Username)
	respondJSON(w, AuthResponse{
		Success: true,
		Token:   token,
		User:    user,
	}, http.StatusOK)
}

// Logout handles session termination
func (a *AuthService) handleLogout(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	token := extractToken(r)
	if token == "" {
		respondJSON(w, AuthResponse{Success: false, Message: "No token provided"}, http.StatusUnauthorized)
		return
	}

	if err := a.db.DeleteSession(token); err != nil {
		log.Printf("[Auth] Logout error: %v", err)
	}

	respondJSON(w, AuthResponse{Success: true}, http.StatusOK)
}

// Me returns current user info
func (a *AuthService) handleMe(w http.ResponseWriter, r *http.Request) {
	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, AuthResponse{Success: false, Message: "Unauthorized"}, http.StatusUnauthorized)
		return
	}

	respondJSON(w, AuthResponse{
		Success: true,
		User:    user,
	}, http.StatusOK)
}

// Middleware to authenticate requests
func (a *AuthService) authMiddleware(next http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		token := extractToken(r)
		if token == "" {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		session, err := a.db.GetSessionByToken(token)
		if err != nil || session == nil {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		user, err := a.db.GetUserByID(session.UserID)
		if err != nil || user == nil {
			http.Error(w, "Unauthorized", http.StatusUnauthorized)
			return
		}

		// Store user in request context (simple approach: add to header)
		r.Header.Set("X-User-ID", fmt.Sprintf("%d", user.ID))
		r.Header.Set("X-Username", user.Username)

		next(w, r)
	}
}

// Helper to extract token from Authorization header
func extractToken(r *http.Request) string {
	auth := r.Header.Get("Authorization")
	if auth == "" {
		return ""
	}
	parts := strings.Split(auth, " ")
	if len(parts) != 2 || parts[0] != "Bearer" {
		return ""
	}
	return parts[1]
}

// Helper to get user from request context
func getUserFromContext(r *http.Request) *User {
	userIDStr := r.Header.Get("X-User-ID")
	username := r.Header.Get("X-Username")
	if userIDStr == "" || username == "" {
		return nil
	}

	var userID int64
	fmt.Sscanf(userIDStr, "%d", &userID)

	return &User{
		ID:       userID,
		Username: username,
	}
}

// Generate cryptographically secure token
func generateSecureToken() string {
	b := make([]byte, 32)
	if _, err := rand.Read(b); err != nil {
		log.Fatalf("Failed to generate token: %v", err)
	}
	return hex.EncodeToString(b)
}

// =========================
// Google OAuth Flow
// =========================

// POST /auth/google/init
// Returns an authUrl and a sessionId for polling
func (a *AuthService) handleGoogleInit(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}
	if a.oauthConfig == nil {
		respondJSON(w, map[string]interface{}{"success": false, "message": "Google OAuth not configured"}, http.StatusBadRequest)
		return
	}

	state := generateSecureToken()
	sessionId := generateSecureToken()

	a.mu.Lock()
	a.pendingStates[state] = sessionId
	a.mu.Unlock()

	authURL := a.oauthConfig.AuthCodeURL(state, oauth2.AccessTypeOffline)
	respondJSON(w, map[string]interface{}{
		"success":   true,
		"authUrl":   authURL,
		"sessionId": sessionId,
	}, http.StatusOK)
}

// GET /auth/google/callback?state=...&code=...
// Handles browser callback and issues a session token, storing it by sessionId
func (a *AuthService) handleGoogleCallback(w http.ResponseWriter, r *http.Request) {
	if a.oauthConfig == nil {
		http.Error(w, "Google OAuth not configured", http.StatusBadRequest)
		return
	}
	q := r.URL.Query()
	state := q.Get("state")
	code := q.Get("code")
	if state == "" || code == "" {
		http.Error(w, "Missing state or code", http.StatusBadRequest)
		return
	}

	a.mu.Lock()
	sessionId, ok := a.pendingStates[state]
	if ok {
		delete(a.pendingStates, state)
	}
	a.mu.Unlock()
	if !ok {
		http.Error(w, "Invalid state", http.StatusBadRequest)
		return
	}

	ctx := r.Context()
	token, err := a.oauthConfig.Exchange(ctx, code)
	if err != nil {
		log.Printf("[Auth] Google token exchange failed: %v", err)
		http.Error(w, "Authentication failed", http.StatusUnauthorized)
		return
	}

	// Fetch userinfo from Google
	userInfoURL, _ := url.Parse("https://www.googleapis.com/oauth2/v3/userinfo")
	params := userInfoURL.Query()
	params.Set("access_token", token.AccessToken)
	userInfoURL.RawQuery = params.Encode()

	resp, err := http.Get(userInfoURL.String())
	if err != nil || resp.StatusCode != http.StatusOK {
		log.Printf("[Auth] Failed to fetch Google userinfo: %v", err)
		http.Error(w, "Authentication failed", http.StatusUnauthorized)
		return
	}
	defer resp.Body.Close()
	var guser struct {
		Sub       string `json:"sub"`
		Email     string `json:"email"`
		Name      string `json:"name"`
		GivenName string `json:"given_name"`
		Picture   string `json:"picture"`
		Verified  bool   `json:"email_verified"`
	}
	json.NewDecoder(resp.Body).Decode(&guser)
	if guser.Email == "" {
		http.Error(w, "Email not provided by Google", http.StatusUnauthorized)
		return
	}

	// Upsert user by email
	user, err := a.db.GetUserByEmail(guser.Email)
	if err != nil {
		log.Printf("[Auth] DB error: %v", err)
		http.Error(w, "Server error", http.StatusInternalServerError)
		return
	}
	if user == nil {
		// Derive a username
		base := guser.Name
		if base == "" {
			// use local part of email
			parts := strings.Split(guser.Email, "@")
			base = parts[0]
		}
		// Ensure unique username; attempt base, then base_1..base_10
		uname := base
		// password_hash for oauth users: sentinel
		passwordHash := "oauth_google"
		// Try to create, if conflict, append suffix
		for i := 0; i < 10; i++ {
			if i > 0 {
				uname = fmt.Sprintf("%s_%d", base, i)
			}
			_, err := a.db.CreateUser(uname, passwordHash, guser.Email)
			if err == nil {
				user, _ = a.db.GetUserByUsername(uname)
				break
			}
		}
		if user == nil {
			http.Error(w, "Failed to create user", http.StatusInternalServerError)
			return
		}
	}

	// Check for existing active sessions
	activeSessions, err := a.db.GetActiveSessionsByUserID(user.ID)
	if err != nil {
		log.Printf("[Auth] Failed to check active sessions: %v", err)
	}
	if len(activeSessions) > 0 {
		log.Printf("[Auth] User %d has %d active session(s), invalidating them", user.ID, len(activeSessions))
		// Invalidate all existing sessions to prevent concurrent logins
		if err := a.db.DeleteAllUserSessions(user.ID); err != nil {
			log.Printf("[Auth] Failed to invalidate old sessions: %v", err)
		}
	}

	// Issue new session token
	sessToken := generateSecureToken()
	expiresAt := timeNow().Add(24 * 7 * timeHour)
	if err := a.db.CreateSession(user.ID, sessToken, expiresAt); err != nil {
		log.Printf("[Auth] Failed to create session: %v", err)
		http.Error(w, "Failed to create session", http.StatusInternalServerError)
		return
	}

	// Store completion
	a.mu.Lock()
	a.completedSessions[sessionId] = sessToken
	a.mu.Unlock()

	// Simple completion page
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	fmt.Fprint(w, `<!doctype html>
<html lang="en">
<head>
	<meta charset="utf-8" />
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<title>Sumo Balls • Login successful</title>
	<style>
		:root {
			--bg: #0f1116;
			--card: #171b22;
			--accent: #5ac8fa;
			--accent-2: #8b5cf6;
			--text: #e9edf5;
			--muted: #9aa3b5;
			--success: #52d273;
		}
		* { box-sizing: border-box; }
		body {
			margin: 0;
			min-height: 100vh;
			display: flex;
			align-items: center;
			justify-content: center;
			background: radial-gradient(circle at 20% 20%, rgba(90,200,250,0.10), transparent 35%),
									radial-gradient(circle at 80% 30%, rgba(139,92,246,0.12), transparent 35%),
									linear-gradient(135deg, #0b0d12 0%, #0f1116 60%, #0c1018 100%);
			color: var(--text);
			font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
			padding: 24px;
		}
		.card {
			background: var(--card);
			border: 1px solid rgba(255,255,255,0.06);
			border-radius: 16px;
			padding: 28px 32px;
			max-width: 460px;
			width: 100%;
			box-shadow: 0 20px 70px rgba(0,0,0,0.35);
			backdrop-filter: blur(4px);
		}
		.title {
			display: flex;
			align-items: center;
			gap: 12px;
			font-size: 22px;
			font-weight: 700;
			margin: 0 0 10px 0;
		}
		.title .dot {
			width: 12px;
			height: 12px;
			border-radius: 50%;
			background: var(--success);
			box-shadow: 0 0 16px rgba(82,210,115,0.65);
		}
		.subtitle {
			margin: 0 0 18px 0;
			color: var(--muted);
			line-height: 1.5;
			font-size: 15px;
		}
		.pill {
			display: inline-flex;
			align-items: center;
			gap: 8px;
			padding: 10px 14px;
			border-radius: 12px;
			background: rgba(90,200,250,0.1);
			border: 1px solid rgba(90,200,250,0.25);
			color: var(--text);
			font-weight: 600;
			margin-bottom: 14px;
		}
		.pill .check {
			width: 18px;
			height: 18px;
			border-radius: 50%;
			background: var(--success);
			display: grid;
			place-items: center;
			color: #0b0d12;
			font-size: 12px;
			font-weight: 800;
			box-shadow: 0 0 10px rgba(82,210,115,0.7);
		}
		.actions {
			margin-top: 16px;
			display: flex;
			gap: 10px;
			flex-wrap: wrap;
		}
		.btn {
			border: none;
			border-radius: 10px;
			padding: 12px 16px;
			cursor: pointer;
			font-weight: 700;
			font-size: 14px;
			color: #0b0d12;
			background: linear-gradient(135deg, var(--accent) 0%, var(--accent-2) 100%);
			box-shadow: 0 10px 30px rgba(90,200,250,0.35);
			transition: transform 120ms ease, box-shadow 120ms ease, opacity 120ms ease;
		}
		.btn:hover { transform: translateY(-1px); box-shadow: 0 12px 36px rgba(90,200,250,0.45); }
		.btn:active { transform: translateY(0px); opacity: 0.92; }
		.note { margin-top: 10px; color: var(--muted); font-size: 14px; }
	</style>
</head>
<body>
	<main class="card">
		<div class="title"><span class="dot"></span><span>Login successful</span></div>
		<div class="subtitle">You’re signed in. You can return to Sumo Balls and continue playing.</div>
		<div class="pill"><span class="check">✓</span><span>Session issued securely</span></div>
		<div class="actions">
			<button class="btn" onclick="window.close();">Close tab</button>
		</div>
		<div class="note">If this tab doesn’t close automatically, you can close it and go back to the game.</div>
	</main>
</body>
</html>`)
}

// GET /auth/google/status?sessionId=...
// Client polls this to receive the token once callback completed
func (a *AuthService) handleGoogleStatus(w http.ResponseWriter, r *http.Request) {
	sessionId := r.URL.Query().Get("sessionId")
	if sessionId == "" {
		respondJSON(w, map[string]interface{}{"success": false, "message": "Missing sessionId"}, http.StatusBadRequest)
		return
	}
	a.mu.Lock()
	token, ok := a.completedSessions[sessionId]
	if ok {
		delete(a.completedSessions, sessionId)
	}
	a.mu.Unlock()
	if !ok {
		respondJSON(w, map[string]interface{}{"success": false, "message": "pending"}, http.StatusOK)
		return
	}
	// Load user
	session, err := a.db.GetSessionByToken(token)
	if err != nil || session == nil {
		respondJSON(w, map[string]interface{}{"success": false, "message": "session-not-found"}, http.StatusInternalServerError)
		return
	}
	user, _ := a.db.GetUserByID(session.UserID)
	respondJSON(w, AuthResponse{Success: true, Token: token, User: user}, http.StatusOK)
}

// getenvDefault returns env value or default
func getenvDefault(key, def string) string {
	v := os.Getenv(key)
	if v == "" {
		return def
	}
	return v
}
