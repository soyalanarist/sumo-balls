#include "CreatePlayerView.h"
#include "network/SocialManager.h"
#include <imgui.h>
#include <SDL2/SDL.h>
#include <iostream>

CreatePlayerView::CreatePlayerView() {
    // Suggest a handle based on username
    const User& user = SocialManager::instance().getCurrentUser();
    if (!user.username.empty()) {
        std::string suggestion = user.username;
        // Truncate if too long
        if (suggestion.length() > 15) {
            suggestion = suggestion.substr(0, 15);
        }
        // Add random suffix
        suggestion += std::to_string(1000 + (rand() % 9000));
        
        strncpy(handleInput, suggestion.c_str(), sizeof(handleInput) - 1);
        handleInput[sizeof(handleInput) - 1] = '\0';
        
        // Check initial suggestion
        checkAvailability(handleInput);
    }
}

void CreatePlayerView::update() {
    // Debounced availability checking
    if (!pendingCheck.empty() && SDL_GetTicks() - lastCheckTime > 500) {
        std::string toCheck = pendingCheck;
        pendingCheck.clear();
        checkAvailability(toCheck);
    }
}

void CreatePlayerView::render() {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImVec2 center = vp ? vp->GetCenter() : ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    float targetWidth = (vp ? vp->Size.x : ImGui::GetIO().DisplaySize.x) * 0.45f;
    if (targetWidth < 420.0f) targetWidth = 420.0f;
    if (targetWidth > 720.0f) targetWidth = 720.0f;
    ImGui::SetNextWindowSize(ImVec2(targetWidth, 0), ImGuiCond_Always);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.13f, 0.16f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.68f, 0.98f, 0.3f));
    
    ImGui::Begin("##HandleSetup", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    
    // Title
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetWindowFontScale(1.8f);
    ImVec2 title_size = ImGui::CalcTextSize("CHOOSE YOUR HANDLE");
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - title_size.x * 1.8f) * 0.5f);
    ImGui::TextUnformatted("CHOOSE YOUR HANDLE");
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Description
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
    ImGui::TextWrapped("Pick a unique username and a 4-character tagline. Together they form your unique identity like username#tagline.");
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Handle input section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
    ImGui::Text("USERNAME");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##handle", handleInput, sizeof(handleInput))) {
        validateInput();
    }
    
    // Character count
    int len = strlen(handleInput);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 0.8f));
    ImGui::Text("%d/20 characters (letters, numbers, underscore)", len);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Tagline input section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.80f, 0.82f, 1.0f));
    ImGui::Text("TAGLINE");
    ImGui::PopStyleColor();
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##tagline", taglineInput, sizeof(taglineInput))) {
        // Convert to uppercase and filter non-alphanumeric
        for (int i = 0; taglineInput[i]; i++) {
            if (isalnum(taglineInput[i])) {
                taglineInput[i] = toupper(taglineInput[i]);
            } else {
                // Remove invalid character
                memmove(&taglineInput[i], &taglineInput[i+1], strlen(&taglineInput[i]));
                i--;
            }
        }
        validateInput();
    }
    
    // Tagline count
    int taglineLen = strlen(taglineInput);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 0.8f));
    ImGui::Text("%d/4 characters (letters and numbers only)", taglineLen);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Preview
    if (len >= 3 && taglineLen == 4) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        ImGui::SetWindowFontScale(1.3f);
        ImVec2 preview_size = ImGui::CalcTextSize("your_handle#XXXX");
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - preview_size.x * 1.3f) * 0.5f);
        ImGui::TextUnformatted(("Your handle: " + std::string(handleInput) + "#" + std::string(taglineInput)).c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }
    
    // Status messages
    if (isChecking) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.70f, 0.70f, 0.72f, 1.0f));
        float time = ImGui::GetTime();
        const char* spinner[] = {"|", "/", "-", "\\"};
        ImGui::Text("%s Checking availability...", spinner[(int)(time * 4) % 4]);
        ImGui::PopStyleColor();
    } else if (!errorMessage.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        ImGui::TextWrapped("%s", errorMessage.c_str());
        ImGui::PopStyleColor();
    } else if (isAvailable && len >= 3) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
        ImGui::Text("Handle is available!");
        ImGui::PopStyleColor();
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    
    // Submit button
    ImGui::BeginDisabled(!isAvailable || isSubmitting || len < 3 || taglineLen != 4);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.42f, 0.75f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.60f, 0.88f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 10.0f));
    
    if (ImGui::Button("Confirm Handle", ImVec2(-1, 0))) {
        submitHandle();
    }
    
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::EndDisabled();
    
    if (isSubmitting) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.68f, 0.98f, 1.0f));
        ImGui::Text("Setting up your profile...");
        ImGui::PopStyleColor();
    }
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Footer
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.62f, 0.6f));
    ImGui::TextWrapped("You can change your handle once per 30 days from the settings menu.");
    ImGui::PopStyleColor();
    
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
}

void CreatePlayerView::validateInput() {
    std::string handle = handleInput;
    std::string tagline = taglineInput;
    
    // Basic validation for handle
    if (handle.length() < 3) {
        errorMessage = "Handle must be at least 3 characters";
        isAvailable = false;
        return;
    }
    
    if (handle.length() > 20) {
        errorMessage = "Handle must be at most 20 characters";
        isAvailable = false;
        return;
    }
    
    // Check for valid characters in handle
    for (char c : handle) {
        if (!isalnum(c) && c != '_') {
            errorMessage = "Handle can only contain letters, numbers, and underscores";
            isAvailable = false;
            return;
        }
    }
    
    // Check start character
    if (handle[0] == '_') {
        errorMessage = "Handle must start with a letter or number";
        isAvailable = false;
        return;
    }
    
    // Validate tagline
    if (tagline.length() < 4) {
        errorMessage = "Tagline must be exactly 4 characters";
        isAvailable = false;
        return;
    }
    
    if (tagline.length() > 4) {
        errorMessage = "Tagline must be exactly 4 characters";
        isAvailable = false;
        return;
    }
    
    for (char c : tagline) {
        if (!isalnum(c)) {
            errorMessage = "Tagline can only contain letters and numbers";
            isAvailable = false;
            return;
        }
    }
    
    // Queue debounced availability check
    errorMessage.clear();
    pendingCheck = handle;
    lastCheckTime = SDL_GetTicks();
}

void CreatePlayerView::checkAvailability(const std::string& handle) {
    isChecking = true;
    errorMessage.clear();
    
    SocialManager::instance().checkHandleAvailability(handle,
        [this](bool available, const std::string& message) {
            isChecking = false;
            isAvailable = available;
            if (!available) {
                errorMessage = message;
            }
        });
}

void CreatePlayerView::submitHandle() {
    std::string handle = handleInput;
    std::string tagline = taglineInput;
    if (handle.empty() || tagline.length() != 4 || !isAvailable) return;
    
    isSubmitting = true;
    errorMessage.clear();
    
    SocialManager::instance().setHandle(handle, tagline,
        [this](const User& user) {
            isSubmitting = false;
            statusMessage = "Handle set successfully!";
            std::cout << "[HandleSetup] Handle set to: " << user.handle << "#" << user.tagline << std::endl;
            // Continue to main menu
            action = ScreenTransition::TO_MAIN_MENU;
        },
        [this](bool, const std::string& message) {
            isSubmitting = false;
            errorMessage = message;
        });
}
