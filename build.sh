#!/bin/bash
set -e

# --- 1. Detect OS ---
OS="$(uname -s)"
case "${OS}" in
    Linux*)     PLATFORM=Linux;;
    Darwin*)    PLATFORM=Mac;;
    CYGWIN*|MINGW32*|MSYS*|MINGW*) PLATFORM=Windows;;
    *)          PLATFORM="Unknown";;
esac

echo "--- Detected Platform: $PLATFORM ---"

# --- 2 & 3. Dependency Check & Installation ---
check_and_install() {
    echo "--- Checking dependencies ---"

    NEEDS_INSTALL=false

    if [[ "$PLATFORM" == "Windows" ]]; then
        # Check for CMake and a Compiler (cl or g++)
        if ! command -v cmake &> /dev/null; then NEEDS_INSTALL=true; fi
        # Windows-specific: check for MSVC or MinGW
        if ! command -v cl &> /dev/null && ! command -v g++ &> /dev/null; then NEEDS_INSTALL=true; fi

        if [ "$NEEDS_INSTALL" = true ]; then
            echo "Installing Windows Build Tools via Winget..."
            winget install -e --id Kitware.CMake
            # This installs the VS 2022 Build Tools (heavy but reliable for C++)
            winget install -e --id Microsoft.VisualStudio.2022.BuildTools --override "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
        fi
    elif [[ "$PLATFORM" == "Linux" ]]; then
        if ! command -v cmake &> /dev/null || (! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null); then
            if [ -f /etc/arch-release ]; then
                sudo pacman -S --needed --noconfirm cmake gcc sdl3
            elif [ -f /etc/debian_version ]; then
                sudo apt-get update
                sudo apt-get install -y cmake build-essential libsdl3-dev
            fi
        fi
    elif [[ "$PLATFORM" == "Mac" ]]; then
        if ! command -v cmake &> /dev/null; then
            brew install cmake sdl3
        fi
    fi
}

check_and_install

# --- 4. Preparing build directory ---
mkdir -p build
cd build

# --- 5. Configuring project ---
if [[ "$PLATFORM" == "Windows" ]]; then
    # -G "Visual Studio 17 2022" is the safest bet for Windows Neovim users
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G "Visual Studio 17 2022"
else
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Link compilation database for Neovim (Linux/Mac use symlink, Windows uses hard copy)
if [[ "$PLATFORM" == "Windows" ]]; then
    cp compile_commands.json ../compile_commands.json 2>/dev/null || true
else
    [ ! -L ../compile_commands.json ] && ln -s build/compile_commands.json ../compile_commands.json 2>/dev/null || true
fi

# --- 6. Building project ---
echo "--- Building project ---"
if [[ "$PLATFORM" == "Windows" ]]; then
    # Windows/MSVC multi-config builds put files in /Debug or /Release
    cmake --build . --config Release --parallel
else
    CPU_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
    cmake --build . --parallel "$CPU_CORES"
fi

echo "--- Build complete! ---"

# --- 7. Launching Logic ---
launch_app() {
    if [[ "$PLATFORM" == "Windows" ]]; then
        echo "--- Launching on Windows ---"
        # Check standard MSVC output path first
        if [ -f "./Release/lxpaint.exe" ]; then
            ./Release/lxpaint.exe &
        else
            ./lxpaint.exe &
        fi
    elif [[ "$PLATFORM" == "Linux" ]]; then
        if command -v hyprctl &> /dev/null && [ -n "$HYPRLAND_INSTANCE_SIGNATURE" ]; then
            hyprctl dispatch workspace 3
            ./lxpaint &
            sleep 0.5
            hyprctl dispatch workspace 2
        elif command -v i3-msg &> /dev/null && i3-msg -t get_outputs &> /dev/null; then
            i3-msg workspace 3
            ./lxpaint &
            sleep 0.5
            i3-msg workspace 2
        else
            ./lxpaint
        fi
    elif [[ "$PLATFORM" == "Mac" ]]; then
        ./lxpaint
    fi
}

launch_app
