# Sumo Balls - Screen Architecture Analysis

## Current Screen Structure

### Screen Hierarchy
```
Screen (abstract base)
├── GameScreen (gameplay)
├── WelcomeScreen (auth)
├── FriendsScreen (social)
├── LobbyScreen (matchmaking)
├── GameEndedScreen (multiplayer results)
└── menus/
    ├── MainMenu (main navigation)
    ├── OptionsMenu (settings)
    ├── PauseMenu (in-game overlay)
    └── GameOverMenu (legacy?)
```

### Screen Types
1. **Full Screens** (`isOverlay() = false`)
   - Replace previous screen in view
   - Examples: GameScreen, MainMenu, WelcomeScreen
   
2. **Overlay Screens** (`isOverlay() = true`)
   - Render on top of previous screen
   - Examples: PauseMenu, OptionsMenu (when opened from game)

## ScreenStack Implementation

### Current Architecture
**File:** `src/core/ScreenStack.h/cpp`

**Key Methods:**
- `push()` - Add screen to top of stack
- `pop()` - Remove top screen
- `clear()` - Remove all screens (quit game)
- `update()` - Update top screen (and overlays below it)
- `render()` - Render all visible screens (skips screens hidden by non-overlays)

**Update Logic:**
```cpp
// Updates from top-down, stopping at first non-overlay
for (int i = size - 1; i >= 0; --i) {
    screens[i]->update();
    if (!screens[i]->isOverlay()) break;
}
```

**Render Logic:**
```cpp
// Renders all screens not hidden by non-overlays above them
for (each screen) {
    if (no non-overlay exists above this screen) {
        render this screen
    }
}
```

### MenuAction System
**File:** `src/screens/menus/MenuAction.h`

Screens communicate with ScreenStack via enum actions:
- `NONE` - No action
- `MAIN_MENU` - Return to main menu
- `START_SINGLEPLAYER` - Start offline game
- `PAUSE` - Push pause menu
- `RESUME` - Pop current overlay
- `OPTIONS` - Push options menu
- `QUIT` - Clear stack (exit game)
- `FRIENDS`, `LOBBIES` - Navigate to social features
- `SET_WINDOWED/FULLSCREEN` - Display settings

**Flow:**
1. Screen sets internal `action` via UI interaction
2. ScreenStack calls `getMenuAction()` after update
3. ScreenStack handles action (push/pop/clear)
4. ScreenStack calls `resetMenuAction()` to clear

## Current Issues & Improvements Needed

### 🔴 Critical Issues

1. **No ESC Key Handling in GameScreen**
   - ESC currently quits entire application (GraphicsContext.cpp:103)
   - Should open PauseMenu instead during gameplay
   - Need context-aware ESC handling

2. **Unsafe ScreenStack Memory Access**
   - ASan reports heap-buffer-overflow in `ScreenStack::update()`
   - Likely caused by incorrect vector iteration or screen lifecycle
   - **HIGH PRIORITY BUG**

3. **MenuAction Processing Timing**
   - Actions processed AFTER screen update completes
   - Can cause one-frame delay in state transitions
   - Should be processed immediately or with better sync

### 🟡 Design Issues

4. **Overlay Detection is Brittle**
   - `isOverlay()` is a boolean flag, no hierarchy info
   - Can't tell if a screen SHOULD be overlay in certain contexts
   - Example: OptionsMenu is overlay from game, full screen from main menu

5. **No Screen State Management**
   - Screens don't know their position in stack
   - Can't query "am I visible?" or "what's below me?"
   - Makes context-dependent behavior difficult

6. **Global ESC Handler is Too Aggressive**
   - GraphicsContext directly quits on ESC
   - Doesn't consult screens first
   - Should be screen-driven, not window-driven

7. **Unclear Screen Ownership**
   - ScreenStack owns screens via unique_ptr (good)
   - But screens can't access stack or sibling screens
   - Limited ability to coordinate transitions

### 🟢 Architecture Strengths

✅ **Clean Separation**
- Screens are independent, testable units
- MenuAction enum provides type-safe communication
- ScreenStack handles all navigation logic

✅ **Overlay System**
- Works well for pause menus and dialogs
- Allows game to keep rendering underneath
- Performance efficient (no redundant updates)

✅ **Memory Safety (mostly)**
- unique_ptr prevents leaks
- RAII cleanup on pop/clear
- Exception-safe (though ASan bug exists)

## Recommended Architecture Improvements

### Phase 1: Critical Fixes (Do Now)
1. **Fix ESC Key Handling**
   - Remove global ESC->quit in GraphicsContext
   - Add `handleInput(SDL_Event)` to Screen interface
   - Let GameScreen handle ESC → PAUSE action
   - Fall back to quit for non-game screens

2. **Fix ASan Bug**
   - Review vector access in ScreenStack::update()
   - Ensure screens aren't accessed after pop/clear
   - Add bounds checking and validation

3. **Improve Action Processing**
   - Process actions before next frame
   - Prevent dangling screen references

### Phase 2: Design Improvements (Later)
4. **Screen Context API**
   ```cpp
   class ScreenContext {
       bool isTopScreen() const;
       bool isVisible() const;
       size_t getStackDepth() const;
       void requestPause();
       void requestPop();
   };
   ```

5. **Event Bubbling**
   - Let top screen handle input first
   - Bubble down to screens below if not handled
   - Allows proper overlay input blocking

6. **State Preservation**
   - Don't destroy game state when paused
   - Consider screen suspension/resume hooks
   - Enable seamless pause/resume flow

## Screen Flow Diagrams

### Current Singleplayer Flow
```
WelcomeScreen (auth)
    ↓ authenticated
MainMenu
    ↓ START_SINGLEPLAYER
GameScreen (singleplayer=true)
    ↓ [ESC not working - BUG]
    ↓ game ends
    ↓ YOU WIN / DEFEATED overlay
    ↓ Play Again → reinit game
    ↓ Main Menu → MAIN_MENU action
MainMenu
```

### Desired Pause Flow
```
GameScreen (playing)
    ↓ ESC pressed
GameScreen (still rendering) + PauseMenu (overlay)
    ↓ Resume → pop PauseMenu
GameScreen (playing)
    ↓ Options from pause
GameScreen + PauseMenu + OptionsMenu (overlay stack)
    ↓ Back → pop OptionsMenu
GameScreen + PauseMenu
```

## Resume Project Quality Standards

### What Makes This Good for Resume

✅ **Clean Architecture**
- Screen abstraction with clear responsibilities
- Action-based communication (not callback spaghetti)
- Separation of concerns (rendering, logic, navigation)

✅ **Memory Safety**
- Smart pointers throughout
- RAII patterns
- No manual new/delete

✅ **Modern C++ Patterns**
- virtual interfaces
- enum class for type safety
- std::unique_ptr for ownership
- Rule of zero (compiler-generated destructors)

### What Needs Improvement for Resume

❌ **Missing Input Architecture**
- No formal input handling layer
- ESC hardcoded in wrong place
- Should demonstrate proper event system

❌ **Incomplete Error Handling**
- ASan bug shows testing gaps
- No defensive programming in vector access
- Missing validation in screen lifecycle

❌ **Limited Documentation**
- No comments explaining screen lifecycle
- No diagrams in code
- Assumptions not documented

### Suggested Resume Talking Points (After Fixes)

1. **"Implemented robust screen stack navigation system"**
   - Supports overlay screens for pause/options
   - Type-safe action-based communication
   - Memory-safe with RAII and smart pointers

2. **"Designed modular input handling with event bubbling"**
   - Context-aware ESC key behavior
   - Top-down event propagation
   - Allows overlays to block input to screens below

3. **"Built flexible game state management"**
   - Seamless pause/resume without state destruction
   - Multi-level overlay support (pause → options)
   - Clean separation of UI navigation and game logic

## Next Steps

**Immediate (this session):**
1. ✅ Add input handling to Screen interface
2. ✅ Implement ESC → Pause in GameScreen
3. ✅ Fix global ESC quit behavior
4. ⬜ Test pause/resume/options flow

**Follow-up (future):**
1. Debug and fix ASan heap-buffer-overflow
2. Add comprehensive screen lifecycle tests
3. Document screen architecture in code
4. Consider state preservation for pause
