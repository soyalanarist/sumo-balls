# Client Authentication & Social Features Implementation

## What Was Added to Client

### 1. AuthScreen
- Login and registration UI
- Text input for username, password, email
- Toggle between login/register modes
- Communicates with coordinator via HTTP
- Stores auth token in Settings upon success

### 2. Updated HttpClient
- Now supports Bearer token authentication
- All authenticated requests automatically add `Authorization: Bearer {token}` header
- Backward compatible (token parameter optional)

### 3. Updated Settings
- `authToken` - stores coordinator session token
- `username` - stores logged-in user's name
- `userID` - stores logged-in user's ID

### 4. Game Flow
```
Game Start
  ↓
[Is authToken empty?]
  ├→ YES: Show AuthScreen (login/register)
  │       ↓
  │       [User enters credentials]
  │       ↓
  │       [HTTP POST to /auth/register or /auth/login]
  │       ↓
  │       [Store token in Settings]
  │       ↓
  │       [Transition to MainMenu]
  │
  └→ NO: Show MainMenu directly
        ↓
        [User can create/join lobbies]
```

## Architecture

### Client Files Created
- `src/screens/AuthScreen.h` - Header
- `src/screens/AuthScreen.cpp` - Implementation

### Client Files Modified
- `src/core/Game.cpp` - Added auth check before MainMenu
- `src/core/Settings.h/cpp` - Added token + username fields
- `src/network/HttpClient.h` - Added token parameter
- `CMakeLists.txt` - Added AuthScreen.cpp to build

### Coordinator Integration
Client makes HTTP calls to:
- `POST /auth/register` - Register new account
- `POST /auth/login` - Login and get token
- All future calls include `Authorization: Bearer {token}` header

## Running the Full Stack

### Terminal 1: Start Coordinator
```bash
./coordinator/run.sh
```
Creates SQLite database `coordinator/coordinator.db` with user/session/friend/lobby tables.

### Terminal 2: Start Game Server
```bash
./scripts/run-server.sh 7777
```

### Terminal 3: Start Client
```bash
./scripts/run-client.sh
```
or directly:
```bash
./build/sumo_balls
```

Client will now:
1. Show AuthScreen (no token yet)
2. Let you register/login
3. Store token in Settings
4. Show MainMenu once authenticated

## Next Steps

The following features are ready to implement (use same pattern):

1. **FriendsScreen**
   - List friends
   - Add friend by username
   - Accept/reject requests
   - Remove friend
   - Endpoints: `/friends/*`

2. **LobbyScreen**
   - Create lobby
   - Join lobby
   - Leave lobby
   - Set ready status
   - Start match
   - Endpoints: `/lobby/*`

3. **Persistence**
   - Save token to local config file
   - Auto-login on next startup (if token still valid)
   - Logout button in menus

## Security Notes

- Tokens are 32-byte cryptographically secure random (bcrypt on backend)
- Stored in memory during session (not persisted yet)
- Sent via HTTP (in production, use HTTPS)
- Backend validates token expiry on every request
- Passwords never sent after registration/login

## Testing

1. Start coordinator: `./coordinator/run.sh`
2. In another terminal, test auth endpoints:
   ```bash
   ./coordinator/test_new_api.sh
   ```
3. Start client to see AuthScreen in action

## Known Limitations

- No websocket support (lobbies use polling)
- No offline mode (auth-required)
- Token not persisted (logout on restart)
- No password reset
- No email verification
