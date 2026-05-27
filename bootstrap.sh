#!/bin/bash
set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

SDL_DIR="$REPO_ROOT/external/SDL"
IMGUI_DIR="$REPO_ROOT/external/imgui"

mkdir -p "$REPO_ROOT/external"

echo "--- Checking SDL ---"

if [ ! -d "$SDL_DIR/.git" ]; then
git clone --depth 1 https://github.com/libsdl-org/SDL.git "$SDL_DIR"
else
echo "SDL already exists"
fi

echo "--- Checking ImGui ---"

if [ ! -d "$IMGUI_DIR/.git" ]; then
git clone --depth 1 https://github.com/ocornut/imgui.git "$IMGUI_DIR"
else
echo "ImGui already exists"
fi

echo "--- Bootstrap complete ---"
