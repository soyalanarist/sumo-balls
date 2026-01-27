#include "LobbyView.h"
#include "ui/components/UIComponents.h"
#include "network/SocialManager.h"
#include <imgui.h>
#include <iostream>

LobbyView::LobbyView() {
    loadLobbies();
}

LobbyView::~LobbyView() = default;

void LobbyView::loadLobbies() {
    loading = true;
    SocialManager::instance().loadLobbies([this](const std::vector<Lobby>& lobbies) {
        availableLobbies = lobbies;
        loading = false;
    });
}

void LobbyView::createLobby() {
    SocialManager::instance().createLobby(
        createMaxPlayers,
        [this](const Lobby& lobby) {
            statusMessage = "Lobby created successfully!";
            showCreateLobbyModal = false;
            // Transition to lobby join screen
        },
        [this](bool success, const std::string& message) {
            statusMessage = "Failed to create lobby: " + message;
        }
    );
}

void LobbyView::joinLobby(const std::string& lobbyId) {
    SocialManager::instance().joinLobby(
        lobbyId,
        [this](const Lobby& lobby) {
            statusMessage = "Joined lobby successfully!";
        },
        [this](bool success, const std::string& message) {
            statusMessage = "Failed to join lobby: " + message;
        }
    );
}

void LobbyView::update() {
    // Auto-load lobbies periodically
    static float loadTimer = 0.0f;
    loadTimer += ImGui::GetIO().DeltaTime;
    if (loadTimer > 5.0f && !loading) {
        loadLobbies();
        loadTimer = 0.0f;
    }
}

void LobbyView::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("##Lobbies", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float window_width = ImGui::GetWindowWidth();
    float window_height = ImGui::GetWindowHeight();
    
    // Gradient background
    ImU32 grad_top = ImGui::GetColorU32(ImVec4(0.10f, 0.12f, 0.16f, 1.0f));
    ImU32 grad_bot = ImGui::GetColorU32(ImVec4(0.06f, 0.07f, 0.09f, 1.0f));
    draw_list->AddRectFilledMultiColor(
        ImVec2(0, 0),
        ImVec2(window_width, window_height),
        grad_top, grad_top, grad_bot, grad_bot
    );
    
    // Header area
    ImGui::SetCursorPosY(30);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(2.0f);
    float title_width = ImGui::CalcTextSize("MULTIPLAYER").x;
    ImGui::SetCursorPosX((window_width - title_width * 2.0f) * 0.5f);
    ImGui::TextUnformatted("MULTIPLAYER");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPosY(100);
    ImGui::SetCursorPosX(40);
    ImGui::BeginGroup();
    
    // Search and filter section
    ImGui::PushItemWidth(300);
    static char search_buf[128] = "";
    ImGui::InputTextWithHint("##search", "Search lobbies...", search_buf, IM_ARRAYSIZE(search_buf));
    ImGui::PopItemWidth();
    
    ImGui::SameLine(400);
    static int game_mode = 0;
    ImGui::SetNextItemWidth(200);
    const char* modes[] = { "Battle Royale", "Team Deathmatch *", "Survival *" };
    ImGui::Combo("##gamemode", &game_mode, modes, 3);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Lobbies list
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.16f, 0.19f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.10f, 0.11f, 0.13f, 1.0f));
    
    ImGui::BeginChild("##LobbiesList", ImVec2(0, window_height - 280), true);
    
    if (loading) {
        ImGui::SetCursorPosX(window_width * 0.5f - 50);
        ImGui::SetCursorPosY(window_height * 0.35f);
        ImGui::TextUnformatted("Loading lobbies...");
    } else if (availableLobbies.empty()) {
        ImGui::SetCursorPosX(window_width * 0.5f - 50);
        ImGui::SetCursorPosY(window_height * 0.35f);
        ImGui::TextUnformatted("No lobbies available");
    } else {
        // Render lobbies from backend
        for (const auto& lobby : availableLobbies) {
            ImGui::PushID(lobby.id.c_str());
            
            // Lobby card background
            ImGui::BeginGroup();
            ImGui::Spacing();
            ImGui::Indent(12.0f);
            
            // Lobby name and players
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::SetWindowFontScale(1.1f);
            ImGui::TextUnformatted(lobby.id.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 1.0f));
            ImGui::Spacing();
            ImGui::Text("Players: %d/%d  |  Mode: Battle Royale  |  State: %s", 
                       lobby.getMemberCount(), lobby.max_players, lobby.state.c_str());
            ImGui::PopStyleColor();
            
            ImGui::Unindent(12.0f);
            ImGui::Spacing();
            ImGui::EndGroup();
            
            ImGui::SameLine(window_width - 215);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
            
            if (!lobby.isFull() && ImGui::Button(("Join##" + lobby.id).c_str(), ImVec2(90, 32))) {
                joinLobby(lobby.id);
            }
            ImGui::PopStyleColor(3);
            
            ImGui::SameLine(0, 8);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
            
            if (ImGui::Button(("Watch##" + lobby.id).c_str(), ImVec2(90, 32))) {
                // Spectate
            }
            ImGui::PopStyleColor(3);
            
            ImGui::Spacing();
            ImGui::PopID();
        }
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    
    ImGui::EndGroup();
    
    // Footer buttons
    ImGui::SetCursorPosY(window_height - 60);
    ImGui::Separator();
    ImGui::Spacing();
    
    const float button_width = 200.0f;
    const float button_spacing = 15.0f;
    
    ImGui::SetCursorPosX((window_width - (button_width * 2 + button_spacing)) * 0.5f);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    
    if (ImGui::Button("Create Lobby", ImVec2(button_width, 40))) {
        showCreateLobbyModal = true;
    }
    
    ImGui::SameLine(0, button_spacing);
    ImGui::PopStyleColor(3);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.20f, 0.23f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    
    if (ImGui::Button("Back", ImVec2(button_width, 40))) {
        action = ScreenTransition::TO_MAIN_MENU;
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();
    
    ImGui::End();
    
    // Create lobby modal
    if (showCreateLobbyModal) {
        ImGui::SetNextWindowPos(ImVec2(window_width * 0.5f - 150, window_height * 0.5f - 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 200));
        if (ImGui::Begin("Create Lobby", &showCreateLobbyModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Max Players:");
            ImGui::SliderInt("##maxplayers", &createMaxPlayers, 2, 6);
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            float dialog_width = ImGui::GetContentRegionAvail().x;
            float button_size = (dialog_width - 8) * 0.5f;
            
            if (ImGui::Button("Create", ImVec2(button_size, 0))) {
                createLobby();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(button_size, 0))) {
                showCreateLobbyModal = false;
            }
            
            ImGui::End();
        }
    }
}

ScreenTransition LobbyView::getTransition() const {
    return action;
}

void LobbyView::resetTransition() {
    action = ScreenTransition::NONE;
}
