# WSL Browser Opening - Troubleshooting Guide

## Overview

When you click "Sign in with Google" in Sumo Balls running on WSL, the game needs to open a URL in your Windows default browser. This document explains how it works and how to fix it if it doesn't.

## How It Works

The C++ client automatically detects if it's running on WSL by checking `/proc/version` for "microsoft" or "WSL":

```cpp
FILE* versionFile = fopen("/proc/version", "r");
if (versionFile) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), versionFile)) {
        std::string versionStr(buffer);
        if (versionStr.find("microsoft") != std::string::npos || 
            versionStr.find("WSL") != std::string::npos) {
            isWSL = true;
        }
    }
    fclose(versionFile);
}
```

### WSL Browser Opening Strategy (Priority Order)

1. **`wslview`** (Recommended, WSL 1/2 standard)
   - Command: `wslview "https://..."`
   - Pros: Built-in WSL tool, most reliable
   - Requires: WSL 1 or WSL 2
   - Prerequisite: Must have `wslview` installed

2. **PowerShell `Start-Process`** (Fallback 1)
   - Command: `powershell.exe -NoProfile -Command "Start-Process 'https://...'"`
   - Pros: Always available on Windows, works from WSL
   - Cons: Slower than wslview

3. **CMD `start`** (Fallback 2)
   - Command: `cmd.exe /d /c start "" "https://..."`
   - Pros: Lightweight, built-in
   - Cons: URL escaping can be tricky

## Verification Steps

### 1. Check WSL Detection

The console output should show:
```
[Auth] Detected WSL environment
[Auth] Opened URL via wslview
```

### 2. Verify wslview is Available

In WSL terminal:
```bash
which wslview
```

Expected output:
```
/usr/bin/wslview
```

If not found, install it:
```bash
# Ubuntu/Debian
sudo apt update && sudo apt install wslu

# Arch
sudo pacman -S wslu
```

### 3. Test wslview Manually

```bash
wslview "https://www.google.com"
```

Should open Google in your Windows default browser.

### 4. Test PowerShell Fallback

```bash
powershell.exe -NoProfile -Command "Start-Process 'https://www.google.com'"
```

Should open Google in Windows browser.

## Common Issues and Fixes

### Issue 1: URL doesn't open, but no error in console

**Likely cause:** WSL detection failed (not actually running on WSL)

**Solution:** 
Check if WSL is detected:
```bash
cat /proc/version | grep -i microsoft
```

Should output something containing "Microsoft" or "WSL".

If empty, you're not on WSL. Try:
```bash
uname -a
```

### Issue 2: "wslview: command not found"

**Solution:**
```bash
sudo apt update && sudo apt install wslu
```

Then try again.

### Issue 3: Browser opens but with garbled URL or 400 error

**Likely cause:** URL escaping issue with special characters

**Debug:**
1. Check console output for the exact URL being used
2. Manually copy the URL and paste in Windows browser
3. If manual works but automatic doesn't, URL escaping is the issue

**Solutions:**
- Try a different fallback method (PowerShell or CMD)
- Check if there are special characters in the URL (shouldn't be for OAuth)

### Issue 4: Timeout waiting for Google login

**Likely cause:** Coordinator not running or not accepting connections

**Debug:**
```bash
# Check if coordinator is running
netstat -tlnp | grep 8888
# or
curl -s http://localhost:8888/auth/google/status?sessionId=test
```

**Solution:**
```bash
cd /home/soyal/sumo-balls/coordinator
source .env  # Load credentials
./run.sh
```

## Manual URL Method

If automatic browser opening consistently fails, you can complete the OAuth flow manually:

1. When you click "Sign in with Google", the game will print a URL to console:
   ```
   [Auth] Warning: Could not open browser. Please visit manually: https://accounts.google.com/o/oauth2/v2/auth?...
   ```

2. Copy this URL

3. Open your Windows browser and paste the URL in address bar

4. Complete Google sign-in

5. Browser will redirect to `http://localhost:8888/auth/google/callback`

6. Game will detect the successful login via polling

## Console Debugging

Enable debug output by looking for these console messages:

```cpp
std::cout << "[Auth] Detected WSL environment" << std::endl;
std::cout << "[Auth] Opened URL via wslview" << std::endl;
std::cout << "[Auth] Opened URL via PowerShell Start-Process" << std::endl;
std::cout << "[Auth] Opened URL via cmd start" << std::endl;
std::cout << "[Auth] Warning: Could not open browser. Please visit manually: " << authUrl << std::endl;
```

## Architecture

```
┌─────────────────┐
│  WSL (Linux)    │
│  ┌───────────┐  │
│  │ C++ Game  │  │
│  └─────┬─────┘  │
│        │        │
│   Detects WSL   │
│        │        │
│    ┌───┴─────────────────────────┐
│    │ 1. Try wslview              │
│    │ 2. Try PowerShell           │
│    │ 3. Try CMD                  │
│    │ 4. Print URL manually       │
│    └───┬─────────────────────────┘
│        │
│   └────┼─────────────────────────────┐
│        │   WSL Interop              │
│        v                            │
└────────┼────────────────────────────┘
         │
         v
    ┌─────────────────────────┐
    │  Windows 11             │
    │  ┌──────────────────┐   │
    │  │  Browser         │   │
    │  │  (Chrome/Edge)   │   │
    │  └──────────────────┘   │
    └─────────────────────────┘
```

## Performance Notes

- `wslview`: ~100-200ms to open browser
- `PowerShell`: ~500-1000ms to open browser
- `CMD`: ~200-300ms to open browser
- Manual: User copy-paste time

## Security Considerations

The OAuth flow uses:
- **State token**: Prevents CSRF attacks (generated randomly)
- **Code exchange**: Backend-to-backend (Client Secret never exposed to client)
- **Token storage**: In client-side Settings object (not persisted)
- **HTTPS**: Google OAuth always uses HTTPS for redirects

WSL browser interop is secure because:
- Commands are executed locally (no network transmission)
- URL is same as what user would paste manually
- Browser authentication happens in Windows, not WSL

## WSL Version Compatibility

| Version | wslview | PowerShell | CMD |
|---------|---------|------------|-----|
| WSL 1   | ✓       | ✓          | ✓   |
| WSL 2   | ✓       | ✓          | ✓   |
| WSLg    | ✓       | ✓          | ✓   |

All versions of WSL support the browser opening methods used by Sumo Balls.

## Testing Checklist

- [ ] `which wslview` returns `/usr/bin/wslview`
- [ ] `wslview "https://google.com"` opens browser
- [ ] `cat /proc/version` contains "microsoft" or "WSL"
- [ ] `netstat -tlnp | grep 8888` shows coordinator listening
- [ ] Console shows `[Auth] Detected WSL environment`
- [ ] Console shows one of the "Opened URL via" messages
- [ ] Browser opens and shows Google sign-in page
- [ ] After sign-in, game shows "Login successful!"

If all checks pass, Google OAuth is working correctly!
