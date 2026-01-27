# AI Difficulty System

## Overview
The game now features a configurable AI difficulty system with 5 difficulty levels that control all AI opponents in singleplayer mode.

## Difficulty Levels
1. **Very Easy** - Slow reactions, low aggression, very cautious
2. **Easy** - Moderate reactions, somewhat passive, cautious
3. **Medium** (default) - Balanced reactions, balanced aggression/caution
4. **Hard** - Fast reactions, high aggression, less cautious
5. **Veteran** - Very fast reactions, extremely aggressive, minimal edge fear (nearly impossible to beat)

## How to Change AI Difficulty

### In-Game (Options Menu)
1. Open the game
2. Click "Options" from the main menu
3. Find the "AI Difficulty" section
4. Select your desired difficulty from the dropdown:
   - Very Easy
   - Easy
   - Medium
   - Hard
   - Veteran
5. Click "Apply" to save
6. Start a new singleplayer game - all 5 AI opponents will use this difficulty

### Programmatically (Settings API)
```cpp
// Set difficulty (0-4)
Settings::setAIDifficulty(2);  // 2 = Medium

// Get current difficulty
int currentDiff = Settings::aiDifficulty;
```

### Config File
Edit `config.json` and add/modify:
```json
{
  "aiDifficulty": 2
}
```
Values: 0=VeryEasy, 1=Easy, 2=Medium, 3=Hard, 4=Veteran

## Technical Details

### Difficulty Parameters
Each level has unique parameters that affect behavior:
- **reactionLag**: Time between AI decisions (lower = harder)
- **aggression**: How aggressively AI chases opponents
- **caution**: How cautiously AI avoids edges
- **burstChance**: Probability of aggressive bursts
- **burstDuration**: Length of aggressive windows
- **edgeThreshold**: When edge avoidance kicks in
- **predictionHorizon**: How far ahead AI predicts targets
- **Behavior weights**: seek, avoid, orbit, wander, dodge

### Behavior System
The AI uses a modular steering behavior system:
1. **Seek**: Chase nearest opponent with optional predictive lead
2. **Avoid**: Pull toward arena center when near edge
3. **Orbit**: Tangential patrol around center for circulatory motion
4. **Wander**: Random noise to prevent predictable movement
5. **Burst**: Temporary aggression windows for pressure

### Customizing Profiles
To adjust difficulty balance, edit the profiles in:
`src/game/controllers/AIController.cpp` - `makeProfile()` function

Example profile (Medium):
```cpp
case DifficultyLevel::Medium:
    return {0.16f,  // reactionLag
            0.55f,  // aggression
            0.55f,  // caution
            0.06f,  // burstChance
            2.3f,   // burstDuration
            0.82f,  // edgeThreshold
            0.32f,  // predictionHorizon
            0.85f,  // seekWeight
            0.95f,  // avoidWeight
            0.42f,  // orbitWeight
            0.30f,  // wanderWeight
            0.30f,  // dodgeWeight
            1.9f,   // jitter
            0.32f}; // wanderStrength
```

## Display Settings
The Options menu now also properly supports:
- **Windowed Mode** (recommended) - Alt+Tab friendly
- **Fullscreen Mode** - Immersive gameplay

Both modes save to config and apply immediately.
