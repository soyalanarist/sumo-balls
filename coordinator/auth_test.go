package main

import (
	"bytes"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestRegisterAndLogin(t *testing.T) {
	db, err := NewDatabase(":memory:")
	if err != nil {
		t.Fatalf("failed to init db: %v", err)
	}
	auth := NewAuthService(db)

	regReq := RegisterRequest{Username: "testuser", Password: "password123", Email: "test@example.com"}
	regBody, _ := json.Marshal(regReq)
	regRecorder := httptest.NewRecorder()
	auth.handleRegister(regRecorder, httptest.NewRequest(http.MethodPost, "/auth/register", bytes.NewReader(regBody)))

	if regRecorder.Code != http.StatusOK {
		t.Fatalf("register returned status %d", regRecorder.Code)
	}
	var regResp AuthResponse
	_ = json.Unmarshal(regRecorder.Body.Bytes(), &regResp)
	if !regResp.Success {
		t.Fatalf("register failed: %s", regResp.Message)
	}

	loginReq := LoginRequest{Username: "testuser", Password: "password123"}
	loginBody, _ := json.Marshal(loginReq)
	loginRecorder := httptest.NewRecorder()
	auth.handleLogin(loginRecorder, httptest.NewRequest(http.MethodPost, "/auth/login", bytes.NewReader(loginBody)))

	if loginRecorder.Code != http.StatusOK {
		t.Fatalf("login returned status %d", loginRecorder.Code)
	}
	var loginResp AuthResponse
	_ = json.Unmarshal(loginRecorder.Body.Bytes(), &loginResp)
	if !loginResp.Success || loginResp.Token == "" {
		t.Fatalf("login failed: %s", loginResp.Message)
	}
}
