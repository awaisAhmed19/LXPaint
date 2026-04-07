#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Create build directory if it doesn't exist
echo "--- Preparing build directory ---"
mkdir -p build
cd build

# Run CMake configuration
echo "--- Configuring project ---"
cmake ..

# Build the project
# This uses the same logic you provided to detect cores on Linux or macOS
echo "--- Building project ---"
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
cmake --build . --parallel "$CPU_CORES"

echo "--- Build complete! ---"
echo "Running your app with: ./lxpaint"
./lxpaint
