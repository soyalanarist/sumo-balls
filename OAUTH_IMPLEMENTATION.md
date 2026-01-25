# Google OAuth Implementation Summary

## What's Been Done

Your Sumo Balls game now has complete Google OAuth support configured for WSL + Windows 11. Here's what was implemented:

### 1. **Coordinator Backend** (Already Implemented)
- Google OAuth 2.0 endpoints: `/auth/google/init`, `/auth/google/callback`, `/auth/google/status`
- Secure token exchange and user creation
- Session management with 7-day expiry

### 2. **Client Frontend** (Already Implemented)
- "Sign in with Google" button in AuthScreen
- Automatic OAuth URL generation and browser opening
- Polling mechanism to detect successful login
- 30-second timeout with clear error messages

### 3. **WSL → Windows Browser Integration** (Already Implemented)
The game automatically detects WSL and uses the following priority:
- **wslview** (preferred) - Opens URL in Windows default browser
- **PowerShell Start-Process** (fallback)
- **CMD start** (last resort)
- **Manual URL** - Falls back to printing URL for manual copy-paste

### 4. **Setup Tools & Documentation** (Just Added)
- ✅ `scripts/setup-oauth.sh` - Interactive script to set up credentials
- ✅ `scripts/quick-start.sh` - One-command to build, configure, and run everything
- ✅ `docs/GOOGLE_OAUTH_SETUP.md` - Complete setup guide with Google Cloud Console steps
- ✅ `docs/WSL_BROWSER_GUIDE.md` - Detailed troubleshooting for WSL browser opening
- ✅ Updated README.md with OAuth documentation

## How to Use

### Quick Start (Recommended)

```bash
cd /home/soyal/sumo-balls
bash scripts/setup-oauth.sh
```

This will:
1. Ask for your Google OAuth credentials
2. Save them to your shell profile or .env file
3. Print next steps

### Manual Setup

If you prefer to set credentials yourself:

```bash
# Set credentials in current shell
export GOOGLE_CLIENT_ID="your_client_id"
export GOOGLE_CLIENT_SECRET="your_client_secret"

# Start coordinator
cd /home/soyal/sumo-balls/coordinator
./run.sh

# In another terminal, build and run client
cd /home/soyal/sumo-balls/build
cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4
./sumo_balls
```

## Getting Google OAuth Credentials

You need credentials from Google Cloud Console. See [GOOGLE_OAUTH_SETUP.md](docs/GOOGLE_OAUTH_SETUP.md) for step-by-step instructions, but briefly:

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Create new project
3. Create OAuth 2.0 credentials (Desktop application)
4. Add redirect URI: `http://localhost:8888/auth/google/callback`
5. Copy Client ID and Client Secret
6. Run `scripts/setup-oauth.sh` and paste them

## Architecture

The flow works as follows:

```
┌──────────────────────────────────────────────────────────┐
│ WSL (Linux)                                              │
│ ┌────────────────────────────────────────────────────┐  │
│ │ C++ Game Client (Sumo Balls)                      │  │
│ │  - Detects WSL environment                        │  │
│ │  - Requests OAuth init from coordinator           │  │
│ │  - Opens Google sign-in URL via wslview           │  │
│ │  - Polls for completion                           │  │
│ └────────────────┬───────────────────────────────────┘  │
│                  │                                       │
│                  │ HTTP to localhost:8888                │
│                  │                                       │
│ ┌────────────────▼───────────────────────────────────┐  │
│ │ Coordinator (Go) - :8888                           │  │
│ │  - Issues OAuth auth codes                        │  │
│ │  - Exchanges code for access token                │  │
│ │  - Fetches user info from Google                  │  │
│ │  - Creates/updates local user account             │  │
│ │  - Issues session auth token to client            │  │
│ │  - Stores token for polling by client             │  │
│ └────────────────┬───────────────────────────────────┘  │
│                  │                                       │
└──────────────────┼───────────────────────────────────────┘
                   │
                   ├─────────────────────────────────────┐
                   │ WSL Interop                         │
                   │ (wslview, PowerShell, or CMD)       │
                   │                                     │
                   v                                     │
    ┌──────────────────────────────────────────┐        │
    │ Windows 11                               │        │
    │ ┌──────────────────────────────────────┐ │        │
    │ │ Windows Browser (Chrome/Edge/Firefox)│ │        │
    │ │  - Opens Google OAuth sign-in page   │ │        │
    │ │  - User completes authentication     │ │        │
    │ │  - Browser redirected to localhost   │ │◄───────┘
    │ └──────────────────────────────────────┘ │
    └──────────────────────────────────────────┘
```

## Key Features

✅ **Automatic WSL Detection** - No configuration needed, just works
✅ **Browser Opening Fallbacks** - Uses best available method on your system
✅ **Secure OAuth Flow** - State tokens, CSRF protection, backend secret handling
✅ **Timeout Handling** - 30-second timeout with clear error messages
✅ **Fallback to Manual** - If browser opening fails, URL is printed for manual entry
✅ **Session Management** - Tokens valid for 7 days with database tracking
✅ **User Creation** - Automatically creates account on first OAuth login
✅ **Local Auth Available** - Falls back to username/password if OAuth unavailable

## Testing

After setup, test the flow:

1. **Start all services:**
   ```bash
   bash scripts/quick-start.sh
   ```

2. **In game menu, click "Sign in with Google"**

3. **Browser opens automatically** (should see one of these in console):
   ```
   [Auth] Opened URL via wslview
   [Auth] Opened URL via PowerShell Start-Process
   [Auth] Opened URL via cmd start
   ```

4. **Complete Google sign-in in browser**

5. **Game detects completion and logs you in automatically**

6. **Console shows:**
   ```
   [Auth] Google login successful: <your_google_email>
   ```

## Troubleshooting

See [WSL_BROWSER_GUIDE.md](docs/WSL_BROWSER_GUIDE.md) for detailed troubleshooting.

Common issues:

| Issue | Solution |
|-------|----------|
| "Google OAuth not configured" | Run `scripts/setup-oauth.sh` and restart coordinator |
| Browser doesn't open | Check if `wslview` is installed: `sudo apt install wslu` |
| "Invalid state" error | Clear browser cookies, restart coordinator, try again |
| "Redirect URI mismatch" | Verify Google Cloud Console has exactly `http://localhost:8888/auth/google/callback` |
| Manual URL method needed | URL will be printed in console for manual copy-paste |

## Files Created/Modified

### New Files
- `scripts/setup-oauth.sh` - Interactive credential setup
- `scripts/quick-start.sh` - One-command build and run
- `docs/GOOGLE_OAUTH_SETUP.md` - Complete setup guide
- `docs/WSL_BROWSER_GUIDE.md` - WSL browser troubleshooting

### Modified Files
- `README.md` - Added authentication section with OAuth documentation

### Existing (Already Implemented)
- `src/screens/AuthScreen.cpp` - OAuth button and flow
- `coordinator/auth.go` - All OAuth endpoints
- `src/utils/HttpClient.h` - HTTP communication

## Next Steps

1. ✅ Run `bash scripts/setup-oauth.sh` to configure credentials
2. ✅ Start coordinator: `cd coordinator && ./run.sh`
3. ✅ Build game: `cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4`
4. ✅ Run game: `./sumo_balls`
5. ✅ Test by clicking "Sign in with Google"

That's it! Your WSL + Windows 11 setup is ready for Google OAuth.
