#!/usr/bin/env bash
# =============================================================================
# build.sh — LXPaint incremental build script
#
# Commands:
#   ./build.sh              — Debug build (default)
#   ./build.sh debug        — Debug build
#   ./build.sh release      — Release build
#   ./build.sh relwithdebinfo — RelWithDebInfo build
#   ./build.sh clean        — Remove build directory
#   ./build.sh run          — Build (Debug) and run
#   ./build.sh install      — Release build + cmake --install
#   ./build.sh uninstall    — Remove installed files via CMake manifest
# =============================================================================
set -euo pipefail

# ── Formatting ────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

log_info() { echo -e "${CYAN}[build]${RESET} $*"; }
log_ok() { echo -e "${GREEN}[build]${RESET} $*"; }
log_warn() { echo -e "${YELLOW}[build]${RESET} $*"; }
log_error() { echo -e "${RED}[build]${RESET} $*" >&2; }
log_section() { echo -e "\n${BOLD}>>> $* <<<${RESET}"; }

die() {
	log_error "$*"
	exit 1
}

# ── Configuration ─────────────────────────────────────────────────────────────
BUILD_DIR="build"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
BINARY_NAME="lxpaint" # must match CMakeLists target name

# Detect CPU core count portably
CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# ── Sanity checks ─────────────────────────────────────────────────────────────
require_cmake() {
	command -v cmake &>/dev/null || die "cmake not found. Run: ./dev-install.sh"
	command -v ninja &>/dev/null || die "ninja not found. Run: ./dev-install.sh"
}

require_project_root() {
	[[ -f CMakeLists.txt ]] ||
		die "CMakeLists.txt not found. Run this script from the project root."
}

# ── Build ─────────────────────────────────────────────────────────────────────
do_build() {
	local build_type="$1"
	local build_subdir="${BUILD_DIR}/${build_type,,}"

	log_section "Configuring: ${build_type}"
	cmake -B "$build_subdir" -G Ninja \
		-DCMAKE_BUILD_TYPE="$build_type" \
		-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_COLOR_DIAGNOSTICS=ON

	# Keep compile_commands.json in project root for tooling (clangd, etc.)
	if [[ -f "${build_subdir}/compile_commands.json" ]]; then
		ln -sf "${build_subdir}/compile_commands.json" compile_commands.json
	fi

	log_section "Building: ${build_type} (${CORES} cores)"
	local start_time
	start_time=$(date +%s)

	cmake --build "$build_subdir" -j "$CORES" -- -k 0

	local end_time elapsed
	end_time=$(date +%s)
	elapsed=$((end_time - start_time))

	log_ok "Build complete in ${elapsed}s → ${build_subdir}/"

	# Print binary info
	local binary="${build_subdir}/${BINARY_NAME}"
	if [[ -f "$binary" ]]; then
		local size
		size=$(du -sh "$binary" | cut -f1)
		log_info "Binary: ${binary} (${size})"
	fi
}

# ── Clean ─────────────────────────────────────────────────────────────────────
do_clean() {
	log_section "Cleaning build directory"
	if [[ -d "$BUILD_DIR" ]]; then
		rm -rf "$BUILD_DIR"
		log_ok "Removed: ${BUILD_DIR}/"
	else
		log_info "Nothing to clean (${BUILD_DIR}/ does not exist)."
	fi
	# Remove symlink if present
	[[ -L compile_commands.json ]] && rm compile_commands.json &&
		log_info "Removed: compile_commands.json"
}

do_run() {
	do_build "Debug"

	local build_subdir="${BUILD_DIR}/debug"
	local binary="${build_subdir}/${BINARY_NAME}"

	[[ -x "$binary" ]] || die "Binary not found: ${binary}"

	log_section "Running ${BINARY_NAME}"

	(
		cd "$build_subdir" || exit 1
		exec "./${BINARY_NAME}"
	)
}

# ── Install ───────────────────────────────────────────────────────────────────
do_install() {
	do_build "Release"
	local build_subdir="${BUILD_DIR}/release"
	log_section "Installing to ${INSTALL_PREFIX}"
	cmake --install "$build_subdir" --prefix "$INSTALL_PREFIX"
	log_ok "Installed to ${INSTALL_PREFIX}"
}

# ── Uninstall ─────────────────────────────────────────────────────────────────
do_uninstall() {
	local manifest="${BUILD_DIR}/release/install_manifest.txt"
	if [[ ! -f "$manifest" ]]; then
		die "No install manifest found at ${manifest}. Was 'install' run first?"
	fi
	log_section "Uninstalling"
	# shellcheck disable=SC2046
	sudo xargs rm -f <"$manifest"
	log_ok "Files listed in manifest removed."
	log_info "Empty directories are not removed automatically."
}

# ── Entry point ───────────────────────────────────────────────────────────────
main() {
	require_project_root

	local cmd="${1:-debug}"

	case "${cmd,,}" in
	debug)
		require_cmake
		do_build "Debug"
		;;
	release)
		require_cmake
		do_build "Release"
		;;
	relwithdebinfo | reldbg)
		require_cmake
		do_build "RelWithDebInfo"
		;;
	clean)
		do_clean
		;;
	run)
		require_cmake
		do_run
		;;
	install)
		require_cmake
		do_install
		;;
	uninstall)
		do_uninstall
		;;
	help | --help | -h)
		echo "Usage: $0 [command]"
		echo ""
		echo "Commands:"
		echo "  (none) / debug     Debug build              [default]"
		echo "  release            Release build"
		echo "  relwithdebinfo     RelWithDebInfo build"
		echo "  clean              Remove build directory"
		echo "  run                Debug build + run"
		echo "  install            Release build + system install"
		echo "  uninstall          Remove previously installed files"
		echo ""
		echo "Environment:"
		echo "  INSTALL_PREFIX     Install prefix (default: /usr/local)"
		;;
	*)
		die "Unknown command: ${cmd}. Run '$0 help' for usage."
		;;
	esac
}

main "$@"
