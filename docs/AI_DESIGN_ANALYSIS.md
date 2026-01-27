# AI Design Analysis for Sumo Balls

## Game Mechanics Overview

### Win Condition
**Last player standing in the arena**

### Core Mechanics
1. **Movement**: Players control direction, physics handles momentum
2. **Collisions**: Players bounce off each other with high restitution (2.15x)
3. **Arena Boundaries**: Exit the arena = elimination
4. **Arena Shrinking**: 
   - Starts at 500 radius
   - Wait 10 seconds
   - Shrinks over 60 seconds to 100 radius (20% of original)
   - Forces players toward center, increases confrontation

### Physics Parameters
- **Player Radius**: 32 units
- **Speed**: 180 base thrust
- **Max Speed**: 620 units/sec
- **Acceleration**: 36
- **Friction**: 0.0015 (minimal drag, momentum-heavy)
- **Restitution**: 2.15 (collisions are explosive)

---

## Strategic Elements for AI

### 1. **Spatial Awareness**
The AI needs to understand:

**Arena Position**
- Distance from center
- Distance from current boundary
- Distance from danger zone (red zone outside safe area)
- Safe vs unsafe positioning

**Player Relationships**
- Distance to nearest opponent
- Distance to farthest opponent
- Number of opponents in each zone (near/mid/far)
- Clustering detection (avoid getting surrounded)

**Danger Assessment**
```
Critical Danger: Distance to boundary < 50 units
High Danger: Distance to boundary < 100 units  
Medium Danger: Distance to boundary < 150 units
Safe: Distance to boundary > 150 units
```

### 2. **Tactical Behaviors**

#### **Positioning Strategy (Intermediate Level)**

**Early Game (0-10s, no shrink)**
- Maintain medium distance from center (250-350 radius)
- Avoid edges preemptively
- Stay mobile, don't camp

**Mid Game (10-40s, active shrink)**
- Move toward center gradually
- Stay ahead of shrinking boundary
- Maintain ~100 unit buffer from danger zone

**Late Game (40-70s, heavy shrink, <200 radius)**
- Fight for center control
- Aggressive positioning
- Accept risk for better position

**End Game (70s+, minimum arena, <150 radius)**
- Pure aggression
- No escape room
- Commit to attacks

#### **Combat Tactics**

**When to Engage (Intermediate Behavior)**
```
Engage if:
  - Enemy is closer to edge than me (push advantage)
  - Enemy is between me and center (clear path)
  - Multiple enemies nearby (create chaos, let them fight)
  - I'm in good position (near center, far from edge)
  
Avoid if:
  - I'm closer to edge than enemy (defensive retreat)
  - Enemy has momentum toward me (dodge/sidestep)
  - I'm outnumbered in local area (reposition)
```

**Attack Patterns**
1. **Ram Attack**: Direct charge at opponent near edge
2. **Angle Attack**: Approach at 45° to push toward edge
3. **Pinball**: Use high restitution to ricochet opponents
4. **Herd**: Push multiple opponents toward each other/edge

**Defensive Patterns**
1. **Retreat to Center**: Move toward arena center when threatened
2. **Sidestep**: Perpendicular movement to avoid head-on collision
3. **Momentum Cancel**: Brief reverse thrust before collision to reduce impact
4. **Use Others**: Position so enemies collide with each other

### 3. **Decision Making Priorities**

**Intermediate AI Priority Stack** (highest to lowest):

1. **Survival - Edge Avoidance** (CRITICAL)
   - If distance to boundary < 80 units: PANIC move toward center
   - If distance to boundary < 150 units: Cautious center bias
   - Always track shrink rate and project position

2. **Arena Shrink Adaptation** (HIGH)
   - Track arenaAge and currentArenaRadius
   - Calculate time until danger zone reaches current position
   - Preemptively move inward with 100+ unit buffer

3. **Threat Assessment** (HIGH)
   - Identify nearest opponent
   - Calculate collision trajectory
   - Evaluate relative edge distance (am I safer than them?)

4. **Positioning** (MEDIUM)
   - Prefer being closer to center than average
   - Avoid corners/edges
   - Maintain space from all opponents when safe

5. **Opportunistic Aggression** (MEDIUM)
   - Attack if opponent is in worse position
   - Create collisions when advantageous
   - Use momentum and physics

6. **Chaos Management** (LOW)
   - When 3+ players nearby, increase mobility
   - Unpredictable movement to avoid being target
   - Let others fight, survive the chaos

---

## AI Implementation Requirements

### Input Data Needed
```cpp
struct AIContext {
    // Self state
    Vec2 myPosition;
    Vec2 myVelocity;
    
    // Arena state
    Vec2 arenaCenter;
    float currentArenaRadius;    // NEW: Current shrunk radius
    float arenaAge;              // NEW: Time elapsed
    
    // Opponents
    vector<Vec2> opponentPositions;
    vector<Vec2> opponentVelocities;  // NEW: For trajectory prediction
    
    // Derived metrics
    float myDistanceToCenter;
    float myDistanceToEdge;      // Uses currentArenaRadius
    Vec2 nearestOpponent;
    float nearestOpponentDistance;
    float nearestOpponentEdgeDistance;
};
```

### Behavioral Modules Needed

**1. Edge Avoidance System**
```cpp
Vec2 getEdgeAvoidanceVector(AIContext& ctx) {
    float distToEdge = ctx.myDistanceToEdge;
    float urgency = 1.0f - (distToEdge / ctx.currentArenaRadius);
    
    if (distToEdge < 80.0f) {
        // PANIC: max thrust toward center
        return normalize(ctx.arenaCenter - ctx.myPosition);
    } else if (distToEdge < 150.0f) {
        // CAUTION: gentle bias toward center
        return normalize(ctx.arenaCenter - ctx.myPosition) * urgency;
    }
    return Vec2(0, 0);  // No edge pressure
}
```

**2. Shrink Prediction System**
```cpp
float getTimeUntilDangerReachesMe(AIContext& ctx) {
    // How long until shrinking boundary reaches my position?
    float distanceFromCenter = ctx.myDistanceToCenter;
    float currentRadius = ctx.currentArenaRadius;
    
    if (distanceFromCenter >= currentRadius) {
        return 0.0f;  // Already in danger!
    }
    
    // Calculate shrink rate (depends on game phase)
    // shrinkRate = (startRadius - minRadius) / shrinkDuration
    float shrinkRate = (500.0f - 100.0f) / 60.0f;  // ~6.67 units/sec
    
    float safetyBuffer = distanceFromCenter - currentRadius;
    return safetyBuffer / shrinkRate;
}
```

**3. Threat Assessment System**
```cpp
struct Threat {
    Vec2 position;
    float danger;  // 0-1 scale
    bool shouldEngage;  // true = attack, false = avoid
};

Threat assessNearestOpponent(AIContext& ctx) {
    Threat t;
    t.position = ctx.nearestOpponent;
    
    float myEdgeDist = ctx.myDistanceToEdge;
    float theirEdgeDist = ctx.nearestOpponentEdgeDistance;
    
    if (myEdgeDist < theirEdgeDist - 50.0f) {
        // They're safer than me - HIGH DANGER, avoid
        t.danger = 0.8f;
        t.shouldEngage = false;
    } else if (myEdgeDist > theirEdgeDist + 50.0f) {
        // I'm safer - OPPORTUNITY, attack
        t.danger = 0.3f;
        t.shouldEngage = true;
    } else {
        // Equal footing - NEUTRAL, tactical decision
        t.danger = 0.5f;
        t.shouldEngage = (ctx.arenaAge > 30.0f);  // More aggressive late game
    }
    
    return t;
}
```

**4. Movement Director (Combines All)**
```cpp
Vec2 AIController::getMovementDirection(float dt, Vec2 myPos, 
                                        vector<Vec2> opponents,
                                        Vec2 arenaCenter, 
                                        float currentRadius,
                                        float arenaAge) {
    
    AIContext ctx = buildContext(myPos, opponents, arenaCenter, 
                                  currentRadius, arenaAge);
    
    Vec2 direction(0, 0);
    
    // 1. Critical survival (highest weight)
    Vec2 edgeAvoid = getEdgeAvoidanceVector(ctx);
    direction += edgeAvoid * 3.0f;  // High priority
    
    // 2. Shrink adaptation
    float timeUntilDanger = getTimeUntilDangerReachesMe(ctx);
    if (timeUntilDanger < 5.0f) {
        Vec2 toCenter = normalize(arenaCenter - myPos);
        float urgency = 1.0f - (timeUntilDanger / 5.0f);
        direction += toCenter * urgency * 2.0f;
    }
    
    // 3. Threat response
    if (opponents.size() > 0) {
        Threat t = assessNearestOpponent(ctx);
        Vec2 toOpponent = normalize(t.position - myPos);
        
        if (t.shouldEngage) {
            // Attack: move toward opponent
            direction += toOpponent * 1.0f;
        } else {
            // Avoid: move away from opponent
            direction += -toOpponent * t.danger * 1.5f;
        }
    }
    
    // 4. Center bias (lowest weight, general positioning)
    Vec2 toCenter = normalize(arenaCenter - myPos);
    float centerWeight = 0.3f * (ctx.arenaAge / 70.0f);  // Increases over time
    direction += toCenter * centerWeight;
    
    return normalize(direction);
}
```

---

## Current AI Problems

### Issues with Existing Implementation

1. **No Arena Shrink Awareness**
   - AI doesn't know about shrinking boundary
   - Doesn't adapt to closing space
   - Can get caught by shrink

2. **Static Behavior Profiles**
   - Difficulty levels are fixed parameters
   - No dynamic adaptation to game state
   - Same behavior early/mid/late game

3. **Poor Collision Tactics**
   - Doesn't consider relative edge distance
   - Doesn't use physics (restitution) strategically
   - No momentum awareness

4. **Limited Spatial Intelligence**
   - Doesn't evaluate "safe" vs "unsafe" positions
   - No understanding of being surrounded
   - Doesn't recognize when to retreat vs attack

5. **No Velocity Consideration**
   - Doesn't track opponent momentum
   - Can't predict collisions
   - Doesn't counter incoming attacks

---

## Proposed AI Overhaul

### Architecture Changes

**Pass Additional Data to AI**
```cpp
// Current signature
Vec2 getMovementDirection(float dt, Vec2 position, 
                         vector<Vec2> otherPositions,
                         Vec2 arenaCenter, float arenaRadius);

// NEW signature  
Vec2 getMovementDirection(float dt, Vec2 position, Vec2 velocity,
                         vector<pair<Vec2, Vec2>> opponents,  // pos + vel
                         Vec2 arenaCenter, 
                         float currentArenaRadius,  // shrunk radius
                         float arenaAge);           // game time
```

**Modular Behavior System**
```cpp
class AIController {
private:
    // Behavior modules
    EdgeAvoidanceModule edgeModule;
    ShrinkAdaptationModule shrinkModule;
    CombatModule combatModule;
    PositioningModule positionModule;
    
    // Weights adjusted by difficulty
    float edgeWeight;
    float shrinkWeight;
    float aggressionWeight;
    float positionWeight;
    
public:
    Vec2 getMovementDirection(...) {
        Vec2 result(0, 0);
        result += edgeModule.compute(...) * edgeWeight;
        result += shrinkModule.compute(...) * shrinkWeight;
        result += combatModule.compute(...) * aggressionWeight;
        result += positionModule.compute(...) * positionWeight;
        return normalize(result);
    }
};
```

### Difficulty Scaling

**Very Easy**
- High edge fear (never gets close)
- Poor combat decisions (attacks randomly)
- Slow shrink adaptation
- Predictable movement

**Easy**
- Moderate edge fear
- Basic combat (attacks if closer to center)
- Decent shrink tracking
- Some randomness

**Medium (Target: Intermediate Human)**
- Smart edge management (takes calculated risks)
- Good combat tactics (relative position awareness)
- Proactive shrink adaptation
- Balanced aggression/defense
- Occasional mistakes (10-15% random decisions)

**Hard**
- Minimal edge fear (uses full arena)
- Advanced combat (momentum prediction)
- Perfect shrink tracking
- Aggressive when advantageous

**Veteran**
- Frame-perfect edge usage
- Predictive combat (reads player momentum)
- Optimal positioning always
- Ruthless aggression
- Near-perfect play

---

## Testing Metrics

### How to Evaluate AI Quality

**Survival Time** (primary metric)
- Medium AI should survive 40-60% as long as player
- Should occasionally win (15-25% win rate in 1v1)

**Edge Deaths** (failure metric)
- Should rarely die to pure edge mistakes
- Medium: <10% of deaths should be edge-only (no collision)

**Combat Effectiveness**
- Should successfully push opponents 30-40% of encounters
- Should avoid being pushed 50-60% of encounters

**Positioning Score**
- Track average distance from optimal position
- Medium AI should average <100 units from ideal spot

**Adaptation Speed**
- How quickly does AI move inward as arena shrinks?
- Medium should stay 100-150 units ahead of danger zone

---

## Implementation Priority

### Phase 1: Critical Fixes (Immediate)
1. ✅ **Implement arena shrinking** (DONE)
2. Pass `currentArenaRadius` and `arenaAge` to AI
3. Add basic shrink avoidance (move toward center when time > 10s)
4. Fix edge avoidance to use `currentArenaRadius` not `arenaRadius`

### Phase 2: Combat Intelligence
1. Pass opponent velocities to AI
2. Implement relative edge distance comparison
3. Add attack/avoid decision based on safety
4. Reduce random wandering in favor of tactical movement

### Phase 3: Advanced Behaviors
1. Momentum prediction (trajectory calculation)
2. Multi-opponent awareness (avoid being surrounded)
3. Dynamic aggression scaling (time-based)
4. Collision angle optimization (push toward edge)

### Phase 4: Polish
1. Difficulty-specific parameter tuning
2. Add slight randomness to Medium (human-like imperfection)
3. Testing and balance iteration
4. Add visual debug mode (show AI decision vectors)

---

## Key Takeaway

**The arena shrinking is THE core mechanic that drives all strategy.**

Without it, the game becomes:
- Passive (no reason to engage)
- Slow (can avoid forever)
- Random (winner determined by luck)

With it:
- Forces confrontation
- Creates urgency
- Rewards positioning
- Enables tactical depth

**The AI MUST understand and respect the shrinking arena to play at intermediate level.**

Current AI is like playing chess without knowing pawns can promote - it's missing a fundamental rule that defines optimal play.
