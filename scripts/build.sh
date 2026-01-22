#!/bin/bash
# Build script for Sumo Balls

# Change to project root
cd "$(dirname "$0")/.." || exit 1

echo "=== Building Sumo Balls ==="
echo ""

# Check for dependencies
echo "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found. Please install:"
    echo "  sudo apt-get install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install:"
    echo "  sudo apt-get install build-essential"
    exit 1
fi

# Check for SFML
if ! ldconfig -p | grep -q libsfml-graphics; then
    echo "Warning: SFML not found. Installing..."
    echo "  sudo apt-get install libsfml-dev"
    echo ""
    read -p "Install now? (y/n) " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        sudo apt-get update
        sudo apt-get install -y libsfml-dev
    else
        echo "Please install SFML manually and re-run this script."
        exit 1
    fi
fi

echo ""
echo "Building..."
echo ""

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. || {
    echo "CMake configuration failed!"
    exit 1
}

# Build with all CPU cores
make -j$(nproc) || {
    echo "Build failed!"
    exit 1
}

cd ..

echo ""
echo "=== Build Complete! ==="
echo ""
echo "Executables created:"
echo "  ./build/sumo_balls        - Game client"
echo "  ./build/sumo_balls_server - Dedicated server"
echo ""
echo "Quick start (online only - start in this order):"
echo "  1. ./coordinator/run.sh     - Start matchmaking coordinator (port 8888)"
echo "  2. ./scripts/run-server.sh  - Start game server (registers with coordinator)"
echo "  3. ./scripts/run-client.sh  - Start client (connects to server)"
echo ""
echo "Note: Coordinator requires Go 1.21+. Each runs independently in its own terminal."
echo ""
