package main

import (
	"encoding/json"
	"log"
	"net/http"
	"regexp"
	"strings"
	"time"
)

// Reserved usernames that cannot be used
var reservedHandles = map[string]bool{
	"admin":       true,
	"moderator":   true,
	"mod":         true,
	"root":        true,
	"system":      true,
	"official":    true,
	"support":     true,
	"staff":       true,
	"sumoballs":   true,
	"sumo":        true,
	"game":        true,
	"server":      true,
	"coordinator": true,
	"bot":         true,
	"null":        true,
	"undefined":   true,
	"anonymous":   true,
	"guest":       true,
	"user":        true,
	"player":      true,
}

// Basic profanity list (expand as needed)
var profanityList = []string{
	"fuck", "shit", "ass", "damn", "bitch", "bastard",
	"cunt", "dick", "cock", "pussy", "fag", "nigger",
	"nazi", "hitler", "rape", "whore", "slut",
}

type UsernameService struct {
	db *Database
}

func NewUsernameService(db *Database) *UsernameService {
	return &UsernameService{db: db}
}

// ValidateHandle checks if a handle meets requirements
func ValidateHandle(handle string) (bool, string) {
	// Length check (3-20 characters)
	if len(handle) < 3 {
		return false, "Handle must be at least 3 characters"
	}
	if len(handle) > 20 {
		return false, "Handle must be at most 20 characters"
	}

	// Character validation (letters, numbers, underscores only)
	validPattern := regexp.MustCompile(`^[a-zA-Z0-9_]+$`)
	if !validPattern.MatchString(handle) {
		return false, "Handle can only contain letters, numbers, and underscores"
	}

	// Must start with a letter or number (not underscore)
	if handle[0] == '_' {
		return false, "Handle must start with a letter or number"
	}

	// Normalize for checks
	normalized := strings.ToLower(handle)

	// Check reserved names
	if reservedHandles[normalized] {
		return false, "This handle is reserved and cannot be used"
	}

	// Check profanity
	for _, word := range profanityList {
		if strings.Contains(normalized, word) {
			return false, "Handle contains inappropriate content"
		}
	}

	return true, ""
}

// ValidateTagline checks if a tagline meets requirements
func ValidateTagline(tagline string) (bool, string) {
	// Length check (exactly 4 characters)
	if len(tagline) != 4 {
		return false, "Tagline must be exactly 4 characters"
	}

	// Character validation (letters and numbers only)
	validPattern := regexp.MustCompile(`^[a-zA-Z0-9]+$`)
	if !validPattern.MatchString(tagline) {
		return false, "Tagline can only contain letters and numbers"
	}

	return true, ""
}

// GenerateSuggestedHandle creates a suggested handle from name/email
func GenerateSuggestedHandle(name, email string) string {
	base := name
	if base == "" && email != "" {
		// Extract local part of email
		parts := strings.Split(email, "@")
		if len(parts) > 0 {
			base = parts[0]
		}
	}

	// Clean up: remove non-alphanumeric except underscores
	reg := regexp.MustCompile(`[^a-zA-Z0-9_]+`)
	base = reg.ReplaceAllString(base, "")

	// Ensure it starts with letter/number
	if len(base) > 0 && base[0] == '_' {
		base = "user" + base
	}

	// Truncate to reasonable length
	if len(base) > 15 {
		base = base[:15]
	}

	// Default if empty
	if base == "" {
		base = "player"
	}

	// Append 4-digit suffix for uniqueness
	suffix := generateNumericSuffix()
	return base + suffix
}

func generateNumericSuffix() string {
	// Generate 4-digit random number
	return generateRandomDigits(4)
}

func generateRandomDigits(n int) string {
	bytes := make([]byte, n)
	for i := range bytes {
		bytes[i] = byte('0' + (time.Now().UnixNano() % 10))
		time.Sleep(time.Nanosecond)
	}
	return string(bytes)
}

// Check handle availability
func (u *UsernameService) handleCheckAvailability(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req HandleAvailabilityRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, HandleAvailabilityResponse{
			Success: false,
			Message: "Invalid request",
		}, http.StatusBadRequest)
		return
	}

	// Validate format
	valid, msg := ValidateHandle(req.Handle)
	if !valid {
		respondJSON(w, HandleAvailabilityResponse{
			Success:   true,
			Available: false,
			Message:   msg,
		}, http.StatusOK)
		return
	}

	// Check database availability
	available, err := u.db.IsHandleAvailable(req.Handle)
	if err != nil {
		log.Printf("[Username] Database error checking availability: %v", err)
		respondJSON(w, HandleAvailabilityResponse{
			Success: false,
			Message: "Server error",
		}, http.StatusInternalServerError)
		return
	}

	message := "Handle is available"
	if !available {
		message = "Handle is already taken"
	}

	respondJSON(w, HandleAvailabilityResponse{
		Success:   true,
		Available: available,
		Message:   message,
	}, http.StatusOK)
}

// Set or update user handle
func (u *UsernameService) handleSetHandle(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	user := getUserFromContext(r)
	if user == nil {
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: "Unauthorized",
		}, http.StatusUnauthorized)
		return
	}

	var req SetHandleRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: "Invalid request",
		}, http.StatusBadRequest)
		return
	}

	// Validate handle
	valid, msg := ValidateHandle(req.Handle)
	if !valid {
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: msg,
		}, http.StatusBadRequest)
		return
	}

	// Validate tagline
	validTagline, msgTagline := ValidateTagline(req.Tagline)
	if !validTagline {
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: msgTagline,
		}, http.StatusBadRequest)
		return
	}

	// Check if handle+tagline combination is available (unless it's their current combination)
	existingAvailable, err := u.db.IsHandleTaglineAvailable(req.Handle, req.Tagline)
	if err != nil {
		log.Printf("[Username] Database error: %v", err)
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: "Server error",
		}, http.StatusInternalServerError)
		return
	}

	// Allow if it's their current handle+tagline
	isCurrent := user.Handle == req.Handle && user.Tagline == req.Tagline
	if !existingAvailable && !isCurrent {
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: "This handle and tagline combination is already taken",
		}, http.StatusConflict)
		return
	}

	// Check rate limit (30 days between changes) - only if changing from existing handle
	if user.Handle != "" && user.HandleLastChanged != nil && !isCurrent {
		daysSince := time.Since(*user.HandleLastChanged).Hours() / 24
		if daysSince < 30 {
			daysRemaining := int(30 - daysSince)
			respondJSON(w, SetHandleResponse{
				Success: false,
				Message: "You can only change your handle once per 30 days. " +
					"Please wait " + string(rune(daysRemaining)) + " more days.",
			}, http.StatusBadRequest)
			return
		}
	}

	// Set the handle and tagline
	if err := u.db.SetUserHandle(user.ID, req.Handle, req.Tagline); err != nil {
		log.Printf("[Username] Failed to set handle: %v", err)
		respondJSON(w, SetHandleResponse{
			Success: false,
			Message: "Failed to set handle",
		}, http.StatusInternalServerError)
		return
	}

	// Get updated user
	updatedUser, _ := u.db.GetUserByID(user.ID)

	log.Printf("[Username] User %d set handle to: %s#%s", user.ID, req.Handle, req.Tagline)
	respondJSON(w, SetHandleResponse{
		Success: true,
		Message: "Handle set successfully",
		User:    updatedUser,
	}, http.StatusOK)
}
