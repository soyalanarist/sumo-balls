#!/bin/bash
# Quick start guide for Sumo Balls with Auth & Lobby System

echo "╔═══════════════════════════════════════════════════════╗"
echo "║  Sumo Balls - Auth & Lobby System - Quick Start      ║"
echo "╚═══════════════════════════════════════════════════════╝"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Run this from project root (/home/soyal/sumo-balls)"
    exit 1
fi

echo "📋 Prerequisites:"
echo "  • Go 1.21+ (for coordinator)"
echo "  • g++ & CMake (for game client)"
echo "  • SFML 2.5+ (for graphics)"
echo ""

echo "═══════════════════════════════════════════════════════"
echo "STEP 1: Build the project"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "$ ./scripts/build.sh"
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

./scripts/build.sh || exit 1

echo ""
echo "✓ Build complete!"
echo ""

echo "═══════════════════════════════════════════════════════"
echo "STEP 2: Start Coordinator (Terminal 1)"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "$ ./coordinator/run.sh"
echo ""
echo "This starts:"
echo "  • HTTP server on port 8888"
echo "  • SQLite database at coordinator/coordinator.db"
echo "  • Auth, friends, and lobby services"
echo ""
echo "⏳ Keep this running in a separate terminal."
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

echo ""
echo "═══════════════════════════════════════════════════════"
echo "STEP 3: Start Game Server (Terminal 2)"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "$ ./scripts/run-server.sh 7777"
echo ""
echo "This starts:"
echo "  • Game server on port 7777"
echo "  • Authoritative physics simulation"
echo "  • Networking for online gameplay"
echo ""
echo "⏳ Keep this running in another separate terminal."
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

echo ""
echo "═══════════════════════════════════════════════════════"
echo "STEP 4: Start Game Client (Terminal 3)"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "$ ./scripts/run-client.sh"
echo ""
echo "This starts:"
echo "  • Game client UI"
echo "  • Auth screen (if not logged in)"
echo "  • Connection to local server"
echo ""
read -p "Start client? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

./scripts/run-client.sh

echo ""
echo "═══════════════════════════════════════════════════════"
echo "WELCOME!"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "You should see:"
echo ""
echo "  1. AuthScreen with login/register"
echo "  2. Enter username, password, email"
echo "  3. Click 'Switch to Register' then 'Register'"
echo "  4. Now authenticated → MainMenu"
echo ""
echo "Next features to implement:"
echo "  ✓ Auth ✓ Coordinator API"
echo "  ○ Friends UI"
echo "  ○ Lobby UI"
echo "  ○ Matchmaking UI"
echo "  ○ In-game chat"
echo ""
echo "For testing (while servers running):"
echo "  $ ./coordinator/test_new_api.sh"
echo ""
echo "Documentation:"
echo "  • docs/PHASE1_COMPLETE_SUMMARY.md"
echo "  • docs/AUTH_LOBBY_IMPLEMENTATION.md"
echo "  • docs/CLIENT_AUTH_IMPLEMENTATION.md"
echo ""
echo "Happy developing! 🎮"
