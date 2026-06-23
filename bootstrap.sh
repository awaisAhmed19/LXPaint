#!/bin/bash
set -e
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SDL_DIR="$REPO_ROOT/external/SDL"
SDL_TTF_DIR="$REPO_ROOT/external/SDL_ttf"
IMGUI_DIR="$REPO_ROOT/external/imgui"
mkdir -p "$REPO_ROOT/external"
echo "--- Checking SDL ---"
if [ ! -d "$SDL_DIR/.git" ]; then
  git clone --depth 1 https://github.com/libsdl-org/SDL.git "$SDL_DIR"
else
  echo "SDL already exists"
fi
echo "--- Checking SDL_ttf ---"
if [ ! -d "$SDL_TTF_DIR/.git" ]; then
  git clone --depth 1 https://github.com/libsdl-org/SDL_ttf.git "$SDL_TTF_DIR"
else
  echo "SDL_ttf already exists"
fi
echo "--- Checking ImGui ---"
if [ ! -d "$IMGUI_DIR/.git" ]; then
  git clone --depth 1 https://github.com/ocornut/imgui.git "$IMGUI_DIR"
else
  echo "ImGui already exists"
fi
echo "--- Bootstrap complete ---"
echo ""
echo "NOTE: this script only clones sources, same as it already did for"
echo "SDL/ImGui — it does not build or install anything. SDL3 and"
echo "SDL3_image on this machine were built+installed manually from their"
echo "external/ clones (cmake --build && cmake --install); SDL3_ttf needs"
echo "the same manual step before 'find_package(SDL3_ttf REQUIRED)' will"
echo "succeed. See the SDL3_ttf build commands in the CMakeLists.txt"
echo "comment / setup notes for the exact invocation."
