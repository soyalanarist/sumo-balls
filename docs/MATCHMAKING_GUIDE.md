# Matchmaking Queue Implementation Guide

## Overview

The matchmaking system now has **two distinct gameplay modes**:

1. **Singleplayer** - Local game with 1 human player + 5 AI opponents
2. **Multiplayer** - Queue-based online matchmaking with 2+ human players

## Menu Structure

### Main Menu
```
┌─────────────────────────────────────┐
│  Singleplayer  │  Multiplayer      │
│  (280x70)      │  (280x70)         │
│  Left Button   │  Right Button     │
├─────────────────────────────────────┤
│         Options                     │
├─────────────────────────────────────┤
│          Quit                       │
└─────────────────────────────────────┘
```

### Singleplayer Flow
1. Click "Singleplayer" button
2. Game starts immediately with 6 players (1 human, 5 AI)
3. Classic gameplay with arena shrinking

### Multiplayer Flow
1. Click "Multiplayer" button
2. Button text changes to "Queue: Xs" (shows elapsed seconds)
3. Client automatically enqueues with coordinator
4. Timer updates every frame (client-side only)
5. When coordinator creates match → client connects and plays

## Queue Time Tracking

### Current Implementation (Client-Side Only)

**Where it's implemented:**
- `src/screens/menus/MainMenu.h`: `queueTimerSec` member variable
- `src/screens/menus/MainMenu.cpp`: `update()` function increments timer and updates button text

**Code:**
```cpp
// In MainMenu::update()
if(inQueue) {
    queueTimerSec += deltaTime.asSeconds();
    int seconds = static_cast<int>(queueTimerSec);
    buttons[1].setText("Queue: " + std::to_string(seconds) + "s");
}
```

**Why client-side?**
- Fast, immediate feedback to player
- No network overhead
- Works even if coordinator disconnects

**Limitations:**
- Resets if client closes
- Lost on game restart
- Only shows time since THIS session

---

## Where Should Queue Time Be Tracked Long-Term?

You asked: *"would this be coordinator side? server side? or something else?"*

Here are the options and recommendations:

### Option 1: Coordinator-Side (⭐ RECOMMENDED)
**Pros:**
- Tracks ALL queue attempts across all clients
- Survives client crashes/restarts
- Can measure queue depth patterns, peak times
- Coordinator already creates matches → easy to log

**Cons:**
- Resets when coordinator restarts
- Requires in-memory metrics or persistent DB
- Need to manage old queue entries cleanup

**Implementation:**
```go
// In coordinator/main.go
type QueueEntry struct {
    PlayerID      string
    EnqueuedAt    time.Time
    DequeueReason string // "matched", "timeout", "cancelled"
    MatchID       string
    Duration      time.Duration
}

var queueHistory []QueueEntry  // In-memory or DB
```

---

### Option 2: Dedicated Metrics Service (⭐ BEST FOR PRODUCTION)
**Pros:**
- Separates concerns (matchmaking vs analytics)
- Survives any component restart
- Scalable across multiple coordinators
- Can query historical data
- Industry standard (Prometheus, ELK Stack)

**Cons:**
- Adds infrastructure complexity
- Requires separate service setup

**Implementation:**
```bash
# Send metrics to metrics server (e.g., Prometheus)
curl -X POST http://metrics-server:8080/queue-event \
  -d '{
    "player_id": "player_123",
    "event": "enqueued",
    "timestamp": 1642790000,
    "queue_position": 5
  }'
```

---

### Option 3: Game Server-Side
**Pros:**
- Server already tracks who plays

**Cons:**
- Doesn't track players who never get matched
- Server restarts lose data
- Doesn't capture queue depth/wait times
- Late in pipeline (after match creation)

**Not recommended for queue metrics.**

---

### Option 4: Database (Persistent)
**Pros:**
- Data survives restarts
- Can query historical analytics
- Generates reports

**Cons:**
- Adds complexity to coordinator
- Need DB setup (SQLite, PostgreSQL)
- Potential performance impact at scale

**Implementation:**
```sql
CREATE TABLE queue_events (
    id INTEGER PRIMARY KEY,
    player_id TEXT,
    event_type TEXT,  -- "enqueued", "matched", "cancelled", "timeout"
    timestamp DATETIME,
    duration_ms INTEGER,
    match_id TEXT,
    queue_position INTEGER
);
```

---

## RECOMMENDED ARCHITECTURE FOR YOUR PROJECT

**For MVP (current state):**
- ✅ Keep client-side timer for immediate feedback
- Add simple **in-memory Coordinator metrics** (Option 1)

**Implementation Steps:**

### Step 1: Add Queue Entry Tracking to Coordinator

```go
// In coordinator/main.go
type QueueMetrics struct {
    TotalEnqueued  int64
    TotalMatched   int64
    AverageWaitMs  int64
    CurrentQueueLen int
}

type Coordinator struct {
    // ... existing fields ...
    metrics QueueMetrics
}

// When enqueuing:
func (c *Coordinator) Enqueue(playerID string) string {
    c.mu.Lock()
    c.metrics.TotalEnqueued++
    c.mu.Unlock()
    // ... rest of logic ...
}

// When creating match:
func (c *Coordinator) CreateMatch(playerIDs []string) (*Match, error) {
    c.mu.Lock()
    c.metrics.TotalMatched += len(playerIDs)
    c.mu.Unlock()
    // ... rest of logic ...
}
```

### Step 2: Add Metrics Endpoint

```go
// In coordinator/main.go
func (c *Coordinator) handleMetrics(w http.ResponseWriter, r *http.Request) {
    c.mu.Lock()
    resp := map[string]interface{}{
        "total_enqueued": c.metrics.TotalEnqueued,
        "total_matched":  c.metrics.TotalMatched,
        "current_queue":  len(c.queuedPlayers),
        "timestamp":      time.Now().Unix(),
    }
    c.mu.Unlock()
    
    w.Header().Set("Content-Type", "application/json")
    json.NewEncoder(w).Encode(resp)
}

// Register handler:
http.HandleFunc("/metrics", c.handleMetrics)
```

### Step 3: Client Tracks Total Time (Optional)

```cpp
// In GameScreen.h
struct QueueStatistics {
    float totalQueueTimeSeconds = 0.f;  // Persisted across sessions
    int gamesPlayed = 0;
    int matchesFound = 0;
};

// In settings or config file:
// {
//   "queueStats": {
//     "totalQueueTime": 1234.5,
//     "gamesPlayed": 12,
//     "matchesFound": 5
//   }
// }
```

---

## Current Button Behavior

### Multiplayer Button States

**Before Click:**
- Text: `"Multiplayer"`
- Color: Default (white)
- Function: Click enqueues player, sets `inQueue = true`

**After Click (While Queued):**
- Text: `"Queue: 0s"` → `"Queue: 1s"` → `"Queue: 2s"` (updates every frame)
- Color: Same (white)
- Function: Button disabled during queue (click doesn't reset)

**When Match Found:**
- Text: Reverts to `"Multiplayer"`
- Client auto-connects to server
- Shows game screen instead of menu

---

## Testing

Two windows running:
- Both show main menu with "Singleplayer" and "Multiplayer" buttons
- Click either button to test:
  - **Singleplayer**: Starts game with 1 human + 5 AI immediately
  - **Multiplayer**: Button shows queue timer, auto-enqueues, connects when match found

---

## Next Steps (Optional Enhancements)

1. **Persistent Queue Stats**: Save to config file
   ```json
   {
     "queueStatistics": {
       "totalTimeQueued": 3456.5,
       "totalGamesPlayed": 12,
       "averageQueueWait": 127.3
     }
   }
   ```

2. **Per-Session Tracking**: Show in Pause menu
   ```
   This Session:
   - Time in Queue: 2m 45s
   - Matches Played: 1
   - Win Rate: N/A
   ```

3. **Coordinator Analytics**: `/metrics` endpoint shows:
   ```json
   {
     "total_players_queued": 1547,
     "total_matches_created": 421,
     "avg_wait_time_ms": 2350,
     "current_queue_length": 12
   }
   ```

4. **Database Integration**: Use SQLite to persist:
   - Queue history per player
   - Match results and duration
   - Player rating/ELO (for future skill-based matchmaking)

---

## Summary

- ✅ **Client**: Timer display on button (already implemented)
- 📊 **Coordinator**: Optional in-memory metrics (simple, MVP-friendly)
- 💾 **Database**: Optional persistent storage (future enhancement)
- 🎯 **Recommendation**: Use Coordinator metrics for basic stats, persist to DB later if needed
