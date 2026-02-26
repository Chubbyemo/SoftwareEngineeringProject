#!/bin/bash

# Exit on error
set -e

echo "=== Building The Braendi Dog Game ==="

# Update package list and install system dependencies
echo "Installing system dependencies..."
sudo apt update
sudo apt install -y clang-format cmake git gnupg g++ clang libgtk-3-dev

# Optional: Install documentation tools
echo "Installing documentation tools..."
sudo apt install -y doxygen graphviz

# Create build directory if it doesn't exist
echo "Setting up build directory..."
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Run CMake
echo "Running CMake..."
cmake ..

# Build the project
echo "Building project..."
make -j$(nproc)

echo "=== Build completed successfully! ==="
echo "Executables are in the build/ directory"