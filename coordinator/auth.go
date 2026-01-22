package main

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strings"

	"golang.org/x/crypto/bcrypt"
)

type AuthService struct {
	db *Database
}

func NewAuthService(db *Database) *AuthService {
	return &AuthService{db: db}
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
