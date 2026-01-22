#!/bin/bash

# Build and run the coordinator

cd "$(dirname "$0")"

echo "[*] Building coordinator..."
go build -o coordinator-bin main.go

if [ $? -ne 0 ]; then
    echo "[!] Build failed"
    exit 1
fi

echo "[✓] Build successful"
echo "[*] Starting coordinator on http://localhost:8888"
echo "[*] Press Ctrl+C to stop"
echo ""

./coordinator-bin
