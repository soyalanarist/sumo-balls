#include "LoginView.h"
#include "core/Settings.h"
#include "network/HttpClient.h"
#include "network/SocialManager.h"
#include "utils/SimpleJson.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iterator>

LoginView::LoginView() = default;

void LoginView::update() {
    if (!autoLoginAttempted) {
        autoLoginAttempted = true;
        attemptCredentialLogin();
    }
    if (googleOAuthInProgress) {
        pollGoogleStatus();
    }
}

void LoginView::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    
    float targetWidth = 540.0f;
    float targetHeight = 640.0f;
    
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(targetWidth, targetHeight), ImGuiCond_Always);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.07f, 0.09f, 0.98f));
    
    ImGui::Begin("##WelcomeScreen", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // Gradient background overlay
    ImU32 grad_top = ImGui::GetColorU32(ImVec4(0.15f, 0.36f, 0.61f, 0.08f));
    ImU32 grad_mid = ImGui::GetColorU32(ImVec4(0.34f, 0.22f, 0.61f, 0.06f));
    ImU32 grad_bot = ImGui::GetColorU32(ImVec4(0.06f, 0.07f, 0.09f, 0.0f));
    
    draw_list->AddRectFilledMultiColor(
        window_pos,
        ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y * 0.5f),
        grad_top, grad_top, grad_mid, grad_mid
    );
    draw_list->AddRectFilledMultiColor(
        ImVec2(window_pos.x, window_pos.y + window_size.y * 0.5f),
        ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y),
        grad_mid, grad_mid, grad_bot, grad_bot
    );
    
    // Accent border
    ImU32 border_color = ImGui::GetColorU32(ImVec4(0.35f, 0.68f, 0.98f, 0.2f));
    draw_list->AddRect(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), 
        border_color, 16.0f, 0, 1.5f);
    
    ImGui::SetCursorPos(ImVec2(40, 60));
    ImGui::BeginGroup();
    
    // Logo/Brand section
    ImGui::Dummy(ImVec2(0, 20));
    
    // Main title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.96f, 0.98f, 1.0f));
    const char* title = "SUMO BALLS";
    // Removed unused variable to silence warning
    ImGui::SetWindowFontScale(1.8f);
    float title_w = ImGui::CalcTextSize(title).x;
    ImGui::SetCursorPosX((targetWidth - title_w * 1.8f) * 0.5f);
    ImGui::Text("%s", title);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Dummy(ImVec2(0, 8));
    
    // Subtitle with gradient effect
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 0.9f));
    const char* subtitle = "COMPETITIVE PHYSICS BATTLE ARENA";
    float sub_w = ImGui::CalcTextSize(subtitle).x;
    ImGui::SetCursorPosX((targetWidth - sub_w) * 0.5f);
    ImGui::Text("%s", subtitle);
    ImGui::PopStyleColor();
    
    ImGui::Dummy(ImVec2(0, 50));
    
    // Feature highlights
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.77f, 0.82f, 0.85f));
    
    auto drawFeature = [&](const char* icon, const char* text) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        ImGui::Text("%s", icon);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("%s", text);
    };
    
    ImGui::SetCursorPosX(80);
    drawFeature("●", "Real-time multiplayer physics");
    ImGui::SetCursorPosX(80);
    drawFeature("●", "Competitive matchmaking");
    ImGui::SetCursorPosX(80);
    drawFeature("●", "Cloud progression sync");
    
    ImGui::PopStyleColor();
    
    ImGui::Dummy(ImVec2(0, 40));
    
    // Main content area
    ImGui::SetCursorPosX(40);
    ImGui::BeginChild("##ContentArea", ImVec2(targetWidth - 80, 180), false, ImGuiWindowFlags_NoScrollbar);
    
    // Error/Status messages
    if (!errorMessage.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.8f, 0.2f, 0.2f, 0.15f));
        ImGui::BeginChild("##ErrorBox", ImVec2(0, 60), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        ImGui::TextWrapped("%s", errorMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Dummy(ImVec2(0, 12));
    } else if (!statusMessage.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.7f, 0.4f, 0.15f));
        ImGui::BeginChild("##StatusBox", ImVec2(0, 60), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.9f, 0.6f, 1.0f));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
        ImGui::TextWrapped("%s", statusMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Dummy(ImVec2(0, 12));
    } else {
        ImGui::Dummy(ImVec2(0, 72));
    }
    
    // Sign-in button
    ImGui::BeginDisabled(isLoading || disableGoogleAuth);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 18.0f));
    
    ImVec4 btn_color = ImVec4(0.35f, 0.68f, 0.98f, 1.0f);
    ImVec4 btn_hover = ImVec4(0.42f, 0.75f, 1.0f, 1.0f);
    ImVec4 btn_active = ImVec4(0.28f, 0.60f, 0.88f, 1.0f);
    
    if (isLoading) {
        btn_color = ImVec4(0.25f, 0.48f, 0.78f, 0.6f);
    }
    
    ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btn_hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, btn_active);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    bool clicked = ImGui::Button(disableGoogleAuth ? "Google Sign-In Disabled" : "Sign in with Google", ImVec2(-1, 0));
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    ImGui::EndDisabled();
    
    if (clicked && !isLoading && !disableGoogleAuth) {
        attemptGoogleLogin();
    }
    
    // Loading indicator
    if (isLoading) {
        ImGui::Dummy(ImVec2(0, 16));
        float time = ImGui::GetTime();
        const char* dots[] = {"   ", ".  ", ".. ", "..."};
        int dot_idx = ((int)(time * 3)) % 4;
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        const char* loading_text = "Authenticating";
        float text_w = ImGui::CalcTextSize((std::string(loading_text) + dots[dot_idx]).c_str()).x;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_w) * 0.5f);
        ImGui::Text("%s%s", loading_text, dots[dot_idx]);
        ImGui::PopStyleColor();
    }
    
    ImGui::EndChild();
    ImGui::EndGroup();
    
    // Footer
    ImGui::SetCursorPos(ImVec2(0, targetHeight - 50));
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 8));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.50f, 0.52f, 0.58f, 0.6f));
    const char* footer = "Secure OAuth 2.0 Authentication";
    float footer_w = ImGui::CalcTextSize(footer).x;
    ImGui::SetCursorPosX((targetWidth - footer_w) * 0.5f);
    ImGui::Text("%s", footer);
    ImGui::PopStyleColor();
    
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void LoginView::attemptCredentialLogin() {
    const char* disableEnv = std::getenv("DISABLE_GOOGLE_AUTH");
    if (disableEnv && std::string(disableEnv) == "1") {
        disableGoogleAuth = true;
        std::cout << "[Auth] Google OAuth disabled via DISABLE_GOOGLE_AUTH env" << std::endl;
    }

    std::string username;
    std::string password;

    const char* userEnv = std::getenv("AUTO_LOGIN_USERNAME");
    const char* passEnv = std::getenv("AUTO_LOGIN_PASSWORD");
    if (userEnv && passEnv) {
        username = userEnv;
        password = passEnv;
        std::cout << "[Auth] Found auto-login credentials in environment" << std::endl;
    }

    if (username.empty() || password.empty()) {
        std::string configPath = "config.json";
        const char* cfgEnv = std::getenv("CONFIG_PATH");
        if (cfgEnv && *cfgEnv) {
            configPath = cfgEnv;
        }

        std::ifstream cfg(configPath);
        if (cfg.good()) {
            std::string content((std::istreambuf_iterator<char>(cfg)), std::istreambuf_iterator<char>());
            SimpleJson cfgJson = SimpleJson::parse(content);
            if (cfgJson.has("username")) {
                username = cfgJson.getString("username");
            }
            if (cfgJson.has("password")) {
                password = cfgJson.getString("password");
            }
            if (cfgJson.has("disableGoogleAuth")) {
                disableGoogleAuth = disableGoogleAuth || cfgJson.getBool("disableGoogleAuth");
            }
            if (cfgJson.has("skipGoogleAuth")) {
                disableGoogleAuth = disableGoogleAuth || cfgJson.getBool("skipGoogleAuth");
                if (cfgJson.getBool("skipGoogleAuth")) {
                    std::cout << "[Auth] Google OAuth disabled via config file" << std::endl;
                }
            }
            if (!username.empty()) {
                std::cout << "[Auth] Found credentials in config: username=" << username << std::endl;
            }
        } else {
            std::cout << "[Auth] Config file not found at: " << configPath << std::endl;
        }
    }

    if (username.empty() || password.empty()) {
        std::cout << "[Auth] No credentials configured; will use Google OAuth" << std::endl;
        return; // No credentials configured; fall back to Google flow
    }

    disableGoogleAuth = true;
    isLoading = true;
    statusMessage = std::string("Signing in as ") + username + "...";
    std::cout << "[Auth] Attempting credential login for: " << username << std::endl;

    SimpleJson request;
    request.set("username", username);
    request.set("password", password);

    std::string response = HttpClient::post("http://localhost:8888/auth/login", request.toString());
    SimpleJson resp = SimpleJson::parse(response);
    if (!resp.getBool("success")) {
        isLoading = false;
        errorMessage = resp.getString("message");
        if (errorMessage.empty()) {
            errorMessage = "Credential login failed.";
        }
        statusMessage.clear();
        std::cout << "[Auth] Credential login failed: " << errorMessage << std::endl;
        return;
    }

    Settings::authToken = resp.getString("token");

    std::string userJsonStr = resp.get("user");
    if (!userJsonStr.empty()) {
        SimpleJson userJson = SimpleJson::parse(userJsonStr);
        Settings::username = userJson.getString("username");
        Settings::userID = userJson.getInt("id");

        User user;
        user.id = userJson.getInt("id");
        user.username = userJson.getString("username");
        user.handle = userJson.getString("handle");
        user.tagline = userJson.getString("tagline");
        user.email = userJson.getString("email");
        user.needs_handle_setup = userJson.getBool("needs_handle_setup");
        user.created_at = userJson.getString("created_at");
        SocialManager::instance().setCurrentUser(user);

        isLoading = false;
        statusMessage = "Login successful!";
        action = user.needs_handle_setup ? ScreenTransition::TO_HANDLE_SETUP : ScreenTransition::TO_MAIN_MENU;
        std::cout << "[Auth] Credential login successful: " << user.username << " (ID: " << user.id << ")" << std::endl;
    } else {
        isLoading = false;
        errorMessage = "Invalid user data received";
        statusMessage.clear();
        std::cout << "[Auth] Credential login: Invalid user data in response" << std::endl;
    }
}

void LoginView::attemptGoogleLogin() {
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

void LoginView::pollGoogleStatus() {
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
        
        // Parse user object (stored as JSON string in SimpleJson)
        std::string userJsonStr = statusJson.get("user");
        if (!userJsonStr.empty()) {
            SimpleJson userJson = SimpleJson::parse(userJsonStr);
            
            Settings::username = userJson.getString("username");
            Settings::userID = userJson.getInt("id");
            
            // Store full user object in SocialManager
            User user;
            user.id = userJson.getInt("id");
            user.username = userJson.getString("username");
            user.handle = userJson.getString("handle");
            user.tagline = userJson.getString("tagline");
            user.email = userJson.getString("email");
            user.needs_handle_setup = userJson.getBool("needs_handle_setup");
            user.created_at = userJson.getString("created_at");
            
            SocialManager::instance().setCurrentUser(user);
            
            googleOAuthInProgress = false;
            isLoading = false;
            statusMessage = "Login successful!";
            std::cout << "[Auth] Google login successful: " << Settings::username << std::endl;
            
            // Check if handle setup is needed
            if (user.needs_handle_setup) {
                std::cout << "[Auth] User needs to set up handle" << std::endl;
                action = ScreenTransition::TO_HANDLE_SETUP;
            } else {
                action = ScreenTransition::TO_MAIN_MENU;
            }
        } else {
            errorMessage = "Invalid user data received";
            googleOAuthInProgress = false;
            isLoading = false;
        }
    }
}
