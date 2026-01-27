#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format not found. Install clang-format." >&2
  exit 1
fi

# Format C++ sources
find src include tests -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
  ! -path "*/build/*" ! -path "*/_deps/*" | xargs -r clang-format -i

# Format Go sources
if command -v gofmt >/dev/null 2>&1; then
  (cd coordinator && gofmt -w .)
fi

echo "Formatting complete."
