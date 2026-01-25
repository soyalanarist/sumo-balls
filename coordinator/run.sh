#!/bin/bash

# Build and run the coordinator

cd "$(dirname "$0")"

echo "[*] Building coordinator..."
go build -o coordinator-bin .

if [ $? -ne 0 ]; then
    echo "[!] Build failed"
    exit 1
fi

echo "[✓] Build successful"
echo "[*] Loading environment (GOOGLE_CLIENT_ID/GOOGLE_CLIENT_SECRET) if present"
if [ -f .env ]; then
    set -a
    source .env
    set +a
    echo "    - Loaded .env"
fi
if [ -z "${GOOGLE_CLIENT_ID}" ] || [ -z "${GOOGLE_CLIENT_SECRET}" ]; then
    echo "[!] Google OAuth not configured (set GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET)"
else
    echo "[✓] Google OAuth configured"
fi
echo "[*] Starting coordinator on http://localhost:8888"
echo "[*] Press Ctrl+C to stop"
echo ""

./coordinator-bin
