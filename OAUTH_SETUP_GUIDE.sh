#!/bin/bash
# Visual setup guide for Google OAuth on WSL

cat << 'EOF'

╔════════════════════════════════════════════════════════════════════════════╗
║                 SUMO BALLS - GOOGLE OAUTH ON WSL + WINDOWS                ║
║                                                                            ║
║                        ✓ SETUP COMPLETE & READY                           ║
╚════════════════════════════════════════════════════════════════════════════╝

📋 WHAT'S BEEN DONE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

✓ Google OAuth endpoints in Coordinator
✓ OAuth button and flow in Game Client  
✓ Automatic WSL detection
✓ Browser opening via wslview (+ PowerShell/CMD fallbacks)
✓ Session polling and user account creation
✓ Setup scripts and comprehensive documentation
✓ Google credentials already configured

📂 NEW FILES CREATED
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

scripts/
  ├─ setup-oauth.sh              Interactive credential setup
  └─ quick-start.sh              Build, configure, and run everything

docs/
  ├─ GOOGLE_OAUTH_SETUP.md       Complete setup guide
  └─ WSL_BROWSER_GUIDE.md        Troubleshooting

  ├─ OAUTH_IMPLEMENTATION.md     Technical summary
  └─ GOOGLE_OAUTH_READY.md       Quick reference (this summary)

📝 QUICK START (5 MINUTES)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Option 1: Automated (Recommended)
─────────────────────────────────
cd /home/soyal/sumo-balls
bash scripts/quick-start.sh
  → Builds everything and starts all services
  → Click "Sign in with Google" in game

Option 2: Manual (Step by step)
──────────────────────────────
Terminal 1 - Start Coordinator:
  cd /home/soyal/sumo-balls/coordinator
  source .env
  ./run.sh

Terminal 2 - Start Game Server:
  cd /home/soyal/sumo-balls/build
  ./sumo_balls_server 9999

Terminal 3 - Start Game Client:
  cd /home/soyal/sumo-balls/build
  ./sumo_balls
  → Click "Sign in with Google"

🌐 HOW IT WORKS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

When you click "Sign in with Google":

  1. Game requests OAuth URL from Coordinator
  2. Game detects WSL environment
  3. Game opens URL in Windows browser via wslview
  4. You complete Google sign-in
  5. Browser redirects back to localhost:8888
  6. Game polls Coordinator for completion
  7. Game receives token and logs you in automatically

NO MANUAL URL COPYING NEEDED! 🎉

🔧 BROWSER OPENING - AUTOMATIC FALLBACKS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

The game tries these methods in order:

  1. wslview          ← Fastest, WSL standard
     └─ If not found, try:
  2. PowerShell       ← Always available
     └─ If fails, try:
  3. CMD start        ← Last resort
     └─ If fails, try:
  4. Manual URL       ← Print URL for copy-paste

Console shows which method was used:
  [Auth] Opened URL via wslview
  [Auth] Opened URL via PowerShell Start-Process
  [Auth] Opened URL via cmd start
  [Auth] Warning: Could not open browser. Please visit manually: https://...

✅ PREREQUISITES CHECK
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Run this to verify everything is ready:

  wslview --version                    # Should show version
  grep -i microsoft /proc/version      # Should show output
  cat /home/soyal/sumo-balls/coordinator/.env | grep GOOGLE
  ls -l /home/soyal/sumo-balls/build/sumo_balls

All should succeed ✓

❌ TROUBLESHOOTING
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

"Google OAuth not configured" → source .env && ./run.sh in coordinator
"Browser doesn't open" → sudo apt install wslu (install wslview)
"Invalid state" error → Clear browser cookies, restart coordinator
Redirect URI mismatch → Check Google Cloud: http://localhost:8888/auth/google/callback

For detailed troubleshooting:
  See: /home/soyal/sumo-balls/docs/WSL_BROWSER_GUIDE.md

📊 ARCHITECTURE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

  WSL (Linux)                              Windows 11
  ┌─────────────────────────────────┐    ┌──────────────────┐
  │ Sumo Balls Client (C++)         │    │ Browser          │
  │                                 │    │ (Chrome/Edge)    │
  │ 1. Click "Sign in with Google"  │    │                  │
  │ 2. Request OAuth init           │    │ 4. Google sign-in│
  │ 3. Detect WSL                   │────│ 5. Authenticate  │
  │ 4. Call wslview                 │────│ 6. Redirect back │
  │ 5. Poll for completion  ←───────┴────│ 7. localhost:8888│
  │ 6. Receive token                │    │                  │
  │ 7. Log in automatically         │    └──────────────────┘
  └─────────────────────────────────┘
           ↓
  ┌─────────────────────────────────┐
  │ Coordinator (Go) :8888          │
  │                                 │
  │ • /auth/google/init             │
  │ • /auth/google/callback         │
  │ • /auth/google/status           │
  └─────────────────────────────────┘

📚 DOCUMENTATION
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Getting Started:
  • Quick reference: /home/soyal/sumo-balls/GOOGLE_OAUTH_READY.md
  • Complete setup: /home/soyal/sumo-balls/docs/GOOGLE_OAUTH_SETUP.md

Troubleshooting:
  • WSL issues: /home/soyal/sumo-balls/docs/WSL_BROWSER_GUIDE.md
  • Implementation details: /home/soyal/sumo-balls/OAUTH_IMPLEMENTATION.md

Game Guide:
  • General info: /home/soyal/sumo-balls/README.md

🚀 NEXT STEPS
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

1. Make sure wslview is installed:
   sudo apt update && sudo apt install wslu

2. Test Google OAuth by running game:
   cd /home/soyal/sumo-balls/build
   cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j4 && ./sumo_balls

3. In game: Click "Sign in with Google"

4. Browser opens automatically → Complete sign-in → Game logs you in

✨ THAT'S IT! YOU'RE READY TO GO! ✨

═══════════════════════════════════════════════════════════════════════════════

Questions? Check:
  • WSL_BROWSER_GUIDE.md for browser opening issues
  • GOOGLE_OAUTH_SETUP.md for credential setup
  • README.md for general game info

Happy gaming! 🎮

EOF
