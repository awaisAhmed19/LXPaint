#!/bin/bash
set -e

echo "--- Preparing build directory ---"
mkdir -p build
cd build

echo "--- Configuring project ---"
cmake ..

echo "--- Building project ---"
CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
cmake --build . --parallel "$CPU_CORES"

echo "--- Build complete! ---"
echo "--- Launching LXPAINT ---"

# 1. Switch focus to Workspace 2 first
hyprctl dispatch workspace 3

# 2. Launch the app (it will open on the active workspace, which is now 2)
./lxpaint &

# 3. Wait a moment to ensure it's up
sleep 0.5

# 4. Switch back to Workspace 1 (your terminal)
hyprctl dispatch workspace 2

echo "--- App is on Workspace 2. Terminal is ready for logs. ---"
