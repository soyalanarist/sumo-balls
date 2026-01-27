#!/bin/bash
# Commit coordinator changes to git

# Change to project root
cd "$(dirname "$0")/.." || exit 1

git add coordinator/ docs/ARCHITECTURE.md docs/COORDINATOR_INTEGRATION.md
git commit -m "Add Go coordinator with queue-based matchmaking and integration guide"
git push origin main
