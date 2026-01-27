#!/usr/bin/env bash
set -euo pipefail

# Bootstrap dependencies and build the project on macOS or Debian/Ubuntu.
# Usage: ./scripts/bootstrap.sh [--release]

BUILD_TYPE="Debug"
if [[ "${1:-}" == "--release" ]]; then
  BUILD_TYPE="Release"
fi

OS="$(uname -s)"
INSTALLER=""
case "$OS" in
  Darwin)
    INSTALLER="brew"
    if ! command -v brew >/dev/null 2>&1; then
      echo "Homebrew not found. Installing Homebrew..."
      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
      eval "$(${HOME}/.brew/bin/brew shellenv 2>/dev/null || /opt/homebrew/bin/brew shellenv)"
    fi
    echo "Installing packages via Homebrew..."
    brew update
    brew install cmake ninja pkg-config sdl2 sdl2_image sdl2_ttf sdl2_mixer go || true
    ;;
  Linux)
    INSTALLER="apt"
    if command -v apt-get >/dev/null 2>&1; then
      echo "Installing packages via apt..."
      sudo apt-get update
      sudo apt-get install -y build-essential cmake ninja-build pkg-config libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev golang clang-format clang-tidy
    else
      echo "Unsupported Linux distribution. Please install: build-essential cmake ninja pkg-config SDL2 (base/image/ttf/mixer), Go, clang-format, clang-tidy."
      exit 1
    fi
    ;;
  *)
    echo "Unsupported OS: $OS"
    exit 1
    ;;
endcase

echo "Configuring build ($BUILD_TYPE)..."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "Building..."
cmake --build build -- -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu)"

echo "Bootstrap complete. Binaries in ./build"
