# Visual Enhancement Guide

## Current State

The Sumo Balls application now features a modern, professional ImGui interface with consistent styling across all screens.

## Design System

### Color Palette

All colors are defined in `src/ui/UIComponents.h` under `UIComponents::Colors`:

- **Accent Blue**: `#4296FB` - Primary action color for buttons and highlights
- **Accent Light**: `#59B0FF` - Hover state for interactive elements
- **Success**: `#66E666` - Positive feedback and confirmation messages
- **Error**: `#FF6666` - Error messages and warnings
- **Warning**: `#FFCC66` - Warning states
- **Text Primary**: `#F3F3F8` - Main text color
- **Text Secondary**: `#B3B3B8` - Subtle text and secondary info
- **BG Dark**: `#141419` - Window backgrounds
- **Surface**: `#1F1F24` - Elevated surfaces (buttons, inputs)

### Typography

- **Headings**: Use `CenteredHeading()` for titles (scale 1.5x)
- **Subheadings**: Use `CenteredSubheading()` for subtitles
- **Body Text**: Use ImGui's default text for content

### Spacing & Layout

- **Window Padding**: 16px (configured in ImGuiManager)
- **Item Spacing**: 12px vertical, 8px horizontal
- **Button Height**: 48px for primary actions, 40px for secondary
- **Button Width**: 280-300px for menu buttons, -1 for full-width

### Styling Constants

- **Border Radius**: 6-8px for buttons and windows
- **Hover Feedback**: Lighter color + scale change when possible
- **Active State**: Darker color to indicate pressed state

## Using the UI Component Library

### Buttons

```cpp
// Primary action buttons (blue, large)
if (UIComponents::PrimaryButton("Sign in with Google")) {
    // Handle action
}

// Secondary buttons (subtle)
if (UIComponents::SecondaryButton("Cancel")) {
    // Handle action
}

// Dangerous actions (red)
if (UIComponents::DangerButton("Delete Account")) {
    // Handle action
}
```

### Text & Headings

```cpp
// Centered heading (large blue text)
UIComponents::CenteredHeading("Game Over");

// Centered subheading (smaller gray text)
UIComponents::CenteredSubheading("Final Score: 1500");

// Status messages with icons
UIComponents::SuccessMessage("Login successful!");
UIComponents::ErrorMessage("Connection failed");
UIComponents::WarningMessage("Low network quality");
UIComponents::InfoMessage("Game starting in 5 seconds");
```

### Decorative Elements

```cpp
// Horizontal line separator
UIComponents::HorizontalSeparator(2.0f, UIComponents::Colors::ACCENT_BLUE);

// Section header with underline
UIComponents::SectionHeader("Game Settings");

// Vertical separator (for side-by-side layouts)
UIComponents::VerticalSeparator();

// Loading animation
UIComponents::LoadingSpinner(30.0f);
```

## Screens to Enhance

### 1. GameScreen
- Add scoreboard with centered heading
- Use SectionHeaders for different stats
- Implement loading spinner for server messages
- Add team colors with custom palette

### 2. LobbyScreen
- List players with better styling
- Use status indicators (Ready, Waiting, Away)
- Implement team selection UI with radio buttons
- Add player search with input styling

### 3. FriendsScreen
- Status badges (Online, Away, Offline)
- Friend list with hover effects
- Action buttons (Invite, Message, Profile)
- Search and filter functionality

### 4. GameEndedScreen
- Large "Game Over" heading
- Victory/Defeat indicator (different colors)
- Detailed stats in organized tables
- Replay button with prominent styling

### 5. OptionsMenu
- Organized sections for different settings
- Toggle switches for yes/no options
- Slider controls for volume/brightness
- Dropdown for graphics quality

### 6. PauseMenu
- Semi-transparent overlay behind menu
- Large "Paused" title
- Resume button (prominent)
- Settings and Quit buttons (secondary)

## Implementation Tips

### 1. Centering Content
```cpp
// For text
float text_width = ImGui::CalcTextSize("Text").x;
ImGui::SetCursorPosX((ImGui::GetWindowWidth() - text_width) * 0.5f);

// For buttons
const float button_width = 280.0f;
ImGui::SetCursorPosX((ImGui::GetWindowWidth() - button_width) * 0.5f);
```

### 2. Custom Colors
```cpp
// Push custom color for text
ImGui::PushStyleColor(ImGuiCol_Text, UIComponents::Colors::ERROR);
ImGui::Text("Error message");
ImGui::PopStyleColor();
```

### 3. Adding Icons
All buttons currently use emoji icons. Some useful ones:
- 🎮 Gameplay
- 👥 Multiplayer/Friends
- ⚙️ Settings
- 🔒 Lock/Auth
- ✓ Success/Checkmark
- ✗ Error/Delete
- ⚠️ Warning
- ℹ️ Info
- ↻ Retry/Refresh
- 🔔 Notifications

### 4. Full-Width Buttons
```cpp
if (UIComponents::PrimaryButton("Button Text", ImVec2(-1, 48))) {
    // Action
}
// The -1 means fill available width
```

### 5. Proper Spacing
```cpp
ImGui::Spacing();  // Single vertical space
ImGui::Spacing();  // Add multiple for more gaps
```

## Performance Notes

- All rendering is done with ImGui's immediate-mode API
- Theme is set once in ImGuiManager::initialize()
- Colors are cached as constants for performance
- Loading spinner uses `ImGui::GetTime()` for smooth animation

## Testing

To see the current visual improvements:

1. Build: `cd build && make -j4`
2. Run: `./sumo_balls`
3. You'll see:
   - Modern dark theme on launch
   - WelcomeScreen with styled Google button
   - MainMenu with emoji icons and decorative elements
   - Professional color transitions on hover

## Future Enhancements

1. **Font Loading**: Load custom fonts for better typography
2. **Animations**: Slide-in animations for screen transitions
3. **Icons**: Replace emoji with SVG icon library
4. **Dark/Light Theme Toggle**: Add theme switcher in OptionsMenu
5. **Sound Feedback**: Audio cues for button clicks
6. **Tooltips**: Hover information for complex buttons
7. **Progress Bars**: For loading and health indicators
8. **Notifications**: Toast messages for system events

## References

- ImGui Documentation: https://github.com/ocornut/imgui
- Current ImGui Version: 1.89.9
- Color Picker Tool: https://colorpicker.fr/
- Modern UI Design Principles: https://material.io/design
