#!/usr/bin/env bash
set -euo pipefail

# Build and run the Go coordinator locally.

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR/coordinator"

echo "Building coordinator..."
go build -o coordinator-bin .

echo "Starting coordinator on :8888 (Ctrl+C to stop)..."
./coordinator-bin
