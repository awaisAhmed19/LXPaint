#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

SDL_DIR="$REPO_ROOT/external/SDL"
IMGUI_DIR="$REPO_ROOT/external/imgui"

echo "--- Checking dependencies ---"

# Clone SDL if missing or empty
if [ ! -d "$SDL_DIR/.git" ] || [ -z "$(ls -A "$SDL_DIR" 2>/dev/null)" ]; then
    echo "--- SDL not found. Cloning SDL3 ---"

    rm -rf "$SDL_DIR"

    git clone --depth 1 https://github.com/libsdl-org/SDL.git "$SDL_DIR"
else
    echo "--- SDL already exists ---"
fi

# Clone ImGui if missing or empty
if [ ! -d "$IMGUI_DIR/.git" ] || [ -z "$(ls -A "$IMGUI_DIR" 2>/dev/null)" ]; then
    echo "--- ImGui not found. Cloning ImGui ---"

    rm -rf "$IMGUI_DIR"

    git clone --depth 1 https://github.com/ocornut/imgui.git "$IMGUI_DIR"
else
    echo "--- ImGui already exists ---"
fi

echo "--- Preparing build directory ---"

mkdir -p "$REPO_ROOT/build"
cd "$REPO_ROOT/build"

echo "--- Configuring project ---"

cmake ..

echo "--- Building project ---"

CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

cmake --build . --parallel "$CPU_CORES"

echo "--- Build complete! ---"
echo "--- Launching LXPaint ---"

./lxpaint

echo "--- LXPaint closed ---"
