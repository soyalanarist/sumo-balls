# Google OAuth Setup Guide for Sumo Balls

## Quick Setup for WSL + Windows 11

This guide walks you through setting up Google OAuth authentication so you can sign in with Google through your Windows default browser.

### Prerequisites

- Google Account
- Access to [Google Cloud Console](https://console.cloud.google.com/)
- WSL 2 with Sumo Balls installed

### Step 1: Create a Google Cloud Project

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Click the project dropdown at the top
3. Click "NEW PROJECT"
4. Name it "Sumo Balls" (or any name you prefer)
5. Click "CREATE"
6. Wait for the project to be created (it may take a minute)

### Step 2: Create OAuth 2.0 Credentials

1. In the Google Cloud Console, go to **APIs & Services** → **Credentials**
2. Click **+ CREATE CREDENTIALS** → **OAuth client ID**
3. If prompted, first click **Configure Consent Screen**:
   - Select **External** user type
   - Click **CREATE**
   - Fill in:
     - **App name**: Sumo Balls
     - **User support email**: your email
     - **Developer contact**: your email
   - Click **SAVE AND CONTINUE**
   - Accept defaults on remaining screens, click **SAVE AND CONTINUE** until done

4. Now create the OAuth credentials:
   - Choose **Desktop application**
   - Click **CREATE**
   - A popup shows your credentials:
     - **Client ID**: (copy this)
     - **Client Secret**: (copy this)
   - Click **OK**

### Step 3: Configure OAuth Redirect URL

1. Still in **Credentials**, find your newly created OAuth 2.0 Client ID
2. Click the pencil icon to edit it
3. Under **Authorized redirect URIs**, click **ADD URI**
4. Add this exact URL: `http://localhost:8888/auth/google/callback`
5. Click **SAVE**

### Step 4: Set Environment Variables for Coordinator

Create a `.env` file in the coordinator directory with your credentials:

```bash
cd /home/soyal/sumo-balls/coordinator
cat > .env << 'EOF'
export GOOGLE_CLIENT_ID="<YOUR_CLIENT_ID>"
export GOOGLE_CLIENT_SECRET="<YOUR_CLIENT_SECRET>"
EOF
chmod +x .env
```

Replace:
- `<YOUR_CLIENT_ID>` with your actual Client ID
- `<YOUR_CLIENT_SECRET>` with your actual Client Secret

### Step 5: Start the Coordinator with OAuth Enabled

```bash
cd /home/soyal/sumo-balls/coordinator
source .env
./run.sh
```

Or manually:

```bash
cd /home/soyal/sumo-balls/coordinator
GOOGLE_CLIENT_ID="<YOUR_CLIENT_ID>" GOOGLE_CLIENT_SECRET="<YOUR_CLIENT_SECRET>" go run *.go
```

### Step 6: Test the OAuth Flow

1. Build and run the Sumo Balls client:
   ```bash
   cd /home/soyal/sumo-balls/build
   cmake -DCMAKE_BUILD_TYPE=Debug ..
   make -j4
   ./sumo_balls
   ```

2. Click **Sign in with Google**
3. Your Windows default browser should open automatically (via `wslview`)
4. Complete the Google sign-in flow
5. You'll be redirected back to the game with your Google account

### Troubleshooting

#### Browser doesn't open automatically

If the browser doesn't open via `wslview`, the OAuth URL will be printed to console. Manually copy and paste it into your Windows browser.

**Check what's printed in console:**
```
[Auth] Opened URL via wslview
```

If you see a warning instead, try these fallbacks:
- PowerShell: `powershell.exe -NoProfile -Command "Start-Process 'URL'"`
- CMD: `cmd.exe /c start "" "URL"`

#### "Google OAuth not configured" error

Make sure:
1. Environment variables are set: `echo $GOOGLE_CLIENT_ID`
2. The coordinator restarted after setting variables
3. Both values are non-empty and correctly copied from Google Cloud Console

#### "Invalid state" error

- Clear browser cookies for localhost:8888
- Restart the coordinator
- Try again

#### Redirect URI mismatch

Ensure the redirect URL in Google Cloud Console is **exactly**:
```
http://localhost:8888/auth/google/callback
```

(Note: `http://` not `https://`, and port `8888` exactly)

### Optional: Store Credentials Permanently

For permanent setup, add to your shell profile (`~/.bashrc` or `~/.zshrc`):

```bash
# Sumo Balls OAuth
export GOOGLE_CLIENT_ID="your_client_id_here"
export GOOGLE_CLIENT_SECRET="your_client_secret_here"
```

Then reload: `source ~/.bashrc`

### Architecture Details

The OAuth flow works as follows:

1. **Client** (C++ game) → **Coordinator** (Go server) requests auth URL
2. **Coordinator** generates OAuth state and returns auth URL to client
3. **Client** opens URL in Windows browser via `wslview`
4. **User** completes Google sign-in in browser
5. **Browser** redirects to `http://localhost:8888/auth/google/callback`
6. **Coordinator** exchanges authorization code for access token
7. **Coordinator** fetches user info from Google
8. **Coordinator** creates local user account and auth token
9. **Client** polls coordinator for completion
10. **Client** receives auth token and user data, logs in locally

The WSL → Windows browser passthrough is handled automatically:
- Client detects WSL environment by reading `/proc/version`
- Client uses `wslview` (WSL 1/2 standard tool) to open URL in Windows
- Fallback: PowerShell `Start-Process` (always available on Windows)
- Last resort: CMD `start` command

### Security Notes

- Google Client Secret is only used on the server (Coordinator), never sent to client
- Auth tokens are issued by the Coordinator, not by Google
- Sessions are validated via the Coordinator's database
- All OAuth state is cryptographically random (256 bits)

### Local Development Alternative

If you prefer not to use Google OAuth for development, you can:
1. Use username/password login (no setup required)
2. Edit [AuthScreen.cpp](../src/screens/AuthScreen.cpp) to hide the Google button

The username/password system is fully functional and doesn't require external services.
