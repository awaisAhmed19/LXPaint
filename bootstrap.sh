#!/usr/bin/env bash

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXTERNAL_DIR="$REPO_ROOT/external"

mkdir -p "$EXTERNAL_DIR"

# ============================================================================
# Dependency versions
# ============================================================================

SDL_VERSION="release-3.4.2"
SDL_IMAGE_VERSION="release-3.4.4"
SDL_TTF_VERSION="release-3.2.2"
IMGUI_VERSION="v1.92.0"

# ============================================================================
# Clone or update helper
# ============================================================================

clone_or_update() {
	local name="$1"
	local repo="$2"
	local version="$3"
	local destination="$4"

	echo "--- Checking ${name} ---"

	if [ ! -d "$destination/.git" ]; then
		echo "Cloning ${name} (${version})..."

		git clone \
			--branch "$version" \
			--depth 1 \
			"$repo" \
			"$destination"

	else
		echo "Updating ${name} -> ${version}..."

		git -C "$destination" fetch origin "$version" --depth 1
		git -C "$destination" checkout "$version"
		git -C "$destination" reset --hard "FETCH_HEAD"
	fi

	echo
}

# ============================================================================
# Bootstrap dependencies
# ============================================================================

clone_or_update \
	"SDL3" \
	"https://github.com/libsdl-org/SDL.git" \
	"$SDL_VERSION" \
	"$EXTERNAL_DIR/SDL"

clone_or_update \
	"SDL3_image" \
	"https://github.com/libsdl-org/SDL_image.git" \
	"$SDL_IMAGE_VERSION" \
	"$EXTERNAL_DIR/SDL_image"

clone_or_update \
	"SDL3_ttf" \
	"https://github.com/libsdl-org/SDL_ttf.git" \
	"$SDL_TTF_VERSION" \
	"$EXTERNAL_DIR/SDL_ttf"

clone_or_update \
	"Dear ImGui" \
	"https://github.com/ocornut/imgui.git" \
	"$IMGUI_VERSION" \
	"$EXTERNAL_DIR/imgui"

echo "=================================================================="
echo "Bootstrap complete."
echo "=================================================================="
echo
echo "Pinned versions:"
echo "  SDL        : ${SDL_VERSION}"
echo "  SDL_image  : ${SDL_IMAGE_VERSION}"
echo "  SDL_ttf    : ${SDL_TTF_VERSION}"
echo "  Dear ImGui : ${IMGUI_VERSION}"
echo
echo "Sources have been cloned/updated only."
echo "No libraries have been built or installed."
echo
echo "Use build.sh (or the Ubuntu Dockerfile) to build the dependencies."
