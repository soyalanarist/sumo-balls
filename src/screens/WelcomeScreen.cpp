#include "WelcomeScreen.h"
#include "../core/Settings.h"
#include "../network/HttpClient.h"
#include "../utils/SimpleJson.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>

WelcomeScreen::WelcomeScreen() = default;

void WelcomeScreen::update() {
    if (googleOAuthInProgress) {
        pollGoogleStatus();
    }
}

void WelcomeScreen::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    float targetWidth = (vp ? vp->Size.x : ImGui::GetIO().DisplaySize.x) * 0.45f;
    if (targetWidth < 420.0f) targetWidth = 420.0f;
    if (targetWidth > 720.0f) targetWidth = 720.0f;
    ImGui::SetNextWindowSize(ImVec2(targetWidth, 0), ImGuiCond_Always);
    
    ImGui::Begin("##Welcome", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Decorative top bar
    ImU32 accent_color = ImGui::GetColorU32(ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + 4), accent_color);
    
    ImGui::Spacing();
    
    // Title with accent color
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    float title_width = ImGui::CalcTextSize("Welcome to Sumo Balls").x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_width) / 2);
    ImGui::TextUnformatted("Welcome to Sumo Balls");
    ImGui::PopFont();
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("⚔️ Battle Online").x) / 2);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 0.8f));
    ImGui::TextUnformatted("⚔️ Battle Online");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Info text with better styling
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
    ImGui::TextWrapped("Sign in with Google to continue playing. Your game progress will be saved to the cloud.");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Error message
    if (!errorMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 8.0f));
        ImGui::TextWrapped("❌ %s", errorMessage.c_str());
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    
    // Status message
    if (!statusMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
        ImGui::TextWrapped("✓ %s", statusMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    
    // Google Sign-In Button with premium styling
    ImGui::Spacing();
    ImGui::BeginDisabled(isLoading);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 14.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.68f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.48f, 0.88f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.95f, 0.97f, 1.0f));
    
    if (ImGui::Button("🔓  Sign in with Google", ImVec2(-1, 48))) {
        if (!isLoading) {
            attemptGoogleLogin();
        }
    }
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    ImGui::EndDisabled();
    
    // Loading animation
    if (isLoading) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.26f, 0.59f, 0.98f, 1.0f));
        float time = ImGui::GetTime();
        const char* spinner[] = {"|", "/", "-", "\\"};
        ImGui::Text("  %s Processing authentication...", spinner[(int)(time * 4) % 4]);
        ImGui::PopStyleColor();
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Footer
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 0.6f));
    float footer_width = ImGui::CalcTextSize("Secure authentication via Google OAuth 2.0").x;
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - footer_width) / 2);
    ImGui::TextUnformatted("Secure authentication via Google OAuth 2.0");
    ImGui::PopStyleColor();
    
    ImGui::End();
}

void WelcomeScreen::attemptGoogleLogin() {
    errorMessage.clear();
    statusMessage.clear();

    SimpleJson request;
    request.set("provider", "google");
    request.set("redirectUri", "http://localhost:8888/auth/google/callback");
    std::string response = HttpClient::post("http://localhost:8888/auth/google/init", request.toString());

    SimpleJson resp = SimpleJson::parse(response);
    if (!resp.getBool("success")) {
        errorMessage = resp.getString("message");
        if (errorMessage.empty()) errorMessage = "Google OAuth not configured.";
        statusMessage.clear();
        return;
    }

    std::string authUrl = resp.getString("authUrl");
    std::string sessionId = resp.getString("sessionId");
    if (authUrl.empty() || sessionId.empty()) {
        errorMessage = "Invalid OAuth initiation response.";
        statusMessage.clear();
        return;
    }

    bool isWSL = false;
    FILE* versionFile = fopen("/proc/version", "r");
    if (versionFile) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), versionFile)) {
            std::string versionStr(buffer);
            if (versionStr.find("microsoft") != std::string::npos || 
                versionStr.find("WSL") != std::string::npos) {
                isWSL = true;
                std::cout << "[Auth] Detected WSL environment" << std::endl;
            }
        }
        fclose(versionFile);
    }

    bool browserOpened = false;

    if (isWSL) {
        std::string cmd = std::string("wslview \"") + authUrl + "\" > /dev/null 2>&1 &";
        int result = std::system(cmd.c_str());
        if (result == 0) {
            std::cout << "[Auth] Opened URL via wslview" << std::endl;
            browserOpened = true;
        }

        if (!browserOpened) {
            std::string escapedUrlPS = authUrl;
            size_t pos = 0;
            while ((pos = escapedUrlPS.find("'", pos)) != std::string::npos) {
                escapedUrlPS.replace(pos, 1, "''");
                pos += 2;
            }
            std::string cmd = "powershell.exe -NoProfile -Command \"Start-Process '" + escapedUrlPS + "'\"";
            int result = std::system(cmd.c_str());
            if (result == 0) {
                std::cout << "[Auth] Opened URL via PowerShell Start-Process" << std::endl;
                browserOpened = true;
            }
        }

        if (!browserOpened) {
            std::string escapedUrlCmd = authUrl;
            size_t p = 0;
            while ((p = escapedUrlCmd.find("&", p)) != std::string::npos) {
                escapedUrlCmd.replace(p, 1, "^&");
                p += 2;
            }
            std::string cmd = "cmd.exe /d /c start \"\" \"" + escapedUrlCmd + "\"";
            int result = std::system(cmd.c_str());
            if (result == 0) {
                std::cout << "[Auth] Opened URL via cmd start" << std::endl;
                browserOpened = true;
            }
        }
    } else {
        std::string cmd = std::string("xdg-open \"") + authUrl + "\" > /dev/null 2>&1 &";
        int result = std::system(cmd.c_str());
        if (result == 0) {
            std::cout << "[Auth] Opened URL via xdg-open" << std::endl;
            browserOpened = true;
        }

        if (!browserOpened) {
            std::string browsers[] = {"sensible-browser", "xdg-open", "google-chrome", "chromium", "chromium-browser", "firefox"};
            for (const auto& browser : browsers) {
                std::string cmd = std::string(browser) + " \"" + authUrl + "\" > /dev/null 2>&1 &";
                int result = std::system(cmd.c_str());
                if (result == 0 || result == 256) {
                    std::cout << "[Auth] Opened URL via: " << browser << std::endl;
                    browserOpened = true;
                    break;
                }
            }
        }
    }
    
    if (!browserOpened) {
        std::cout << "[Auth] Warning: Could not open browser. Please visit manually: " << authUrl << std::endl;
    }

    googleOAuthInProgress = true;
    googleSessionId = sessionId;
    googleOAuthStartTime = SDL_GetTicks();
    googleLastPollTime = googleOAuthStartTime;
    isLoading = true;
    statusMessage = "Opening Google authentication...";
}

void WelcomeScreen::pollGoogleStatus() {
    uint32_t now = SDL_GetTicks();
    
    if (now - googleOAuthStartTime > googleOAuthTimeout) {
        googleOAuthInProgress = false;
        isLoading = false;
        errorMessage = "Google login timed out. Please try again.";
        statusMessage.clear();
        std::cout << "[Auth] Google login timed out after " << (now - googleOAuthStartTime) << "ms" << std::endl;
        return;
    }
    
    if (now - googleLastPollTime < googlePollInterval) {
        return;
    }
    
    googleLastPollTime = now;
    statusMessage = "Waiting for Google login...";
    
    std::string statusResp = HttpClient::get(std::string("http://localhost:8888/auth/google/status?sessionId=") + googleSessionId);
    SimpleJson statusJson = SimpleJson::parse(statusResp);
    
    if (statusJson.getBool("success")) {
        Settings::authToken = statusJson.getString("token");
        Settings::username = statusJson.getString("username");
        Settings::userID = statusJson.getInt("id");
        
        googleOAuthInProgress = false;
        isLoading = false;
        statusMessage = "Login successful!";
        std::cout << "[Auth] Google login successful: " << Settings::username << std::endl;
        action = MenuAction::MAIN_MENU;
    }
}
