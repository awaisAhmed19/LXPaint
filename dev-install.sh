#!/usr/bin/env bash
# =============================================================================
# dev-install.sh — LXPaint developer environment installer
#
# Sets up a complete build environment: compiler, CMake, Ninja, Git,
# SDL3 development headers, pkg-config, and optional debug tools.
# Then optionally clones the repo, configures, and builds.
#
# Usage:
#   ./dev-install.sh             # interactive
#   ./dev-install.sh --yes       # non-interactive
#   ./dev-install.sh --no-build  # install deps only, skip build
# =============================================================================
set -euo pipefail

# ── Formatting ────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

log_info() { echo -e "${CYAN}[INFO]${RESET}  $*"; }
log_ok() { echo -e "${GREEN}[OK]${RESET}    $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${RESET}  $*"; }
log_error() { echo -e "${RED}[ERROR]${RESET} $*" >&2; }
log_section() { echo -e "\n${BOLD}=== $* ===${RESET}"; }
log_step() { echo -e "${BOLD}>>> $*${RESET}"; }

die() {
	log_error "$*"
	exit 1
}

# ── Options ───────────────────────────────────────────────────────────────────
AUTO_YES=false
SKIP_BUILD=false
REPO_URL="https://github.com/awaisAhmed19/lxpaint.git" # update this
BUILD_DIR="build"

for arg in "$@"; do
	case "$arg" in
	--yes | -y) AUTO_YES=true ;;
	--no-build) SKIP_BUILD=true ;;
	--help | -h)
		echo "Usage: $0 [--yes] [--no-build]"
		echo "  --yes        Accept all prompts automatically"
		echo "  --no-build   Install dependencies only, do not build"
		exit 0
		;;
	*) die "Unknown argument: $arg" ;;
	esac
done

confirm() {
	if "$AUTO_YES"; then return 0; fi
	read -rp "$1 [y/N] " ans
	[[ "$ans" =~ ^[Yy]$ ]]
}

# ── Distribution detection ────────────────────────────────────────────────────
detect_distro() {
	[[ -f /etc/os-release ]] || die "/etc/os-release not found."
	local ID="" ID_LIKE=""
	# shellcheck disable=SC1091
	eval "$(grep -E '^(ID|ID_LIKE)=' /etc/os-release | sed 's/"//')"
	DISTRO_ID="${ID,,}"
	DISTRO_LIKE="${ID_LIKE,,}"
	log_info "Detected: ${ID} (family: ${DISTRO_LIKE:-none})"
}

# ── Package resolution ────────────────────────────────────────────────────────
# Sets globals: PKG_MANAGER, INSTALL_CMD
# Sets arrays:  TOOL_PKGS, SDL_PC_NAMES
#
# TOOL_PKGS    — one package name per build tool / compiler / debug utility.
#                Each entry maps to exactly one check in check_dependencies().
#
# SDL_PC_NAMES — the pkg-config module names to use for SDL3, SDL3_image,
#                and SDL3_ttf on this distro (in that order). These are the
#                same names CMakeLists.txt probes, so if they pass here they
#                will pass during cmake configuration too.
resolve_packages() {
	local id="$DISTRO_ID"
	local like="$DISTRO_LIKE"

	resolve_for() {
		case "$1" in
		arch)
			PKG_MANAGER="pacman"
			INSTALL_CMD=(sudo pacman -S --needed --noconfirm)
			# Arch bundles headers in the main package (no -dev split).
			TOOL_PKGS=(base-devel cmake ninja git pkgconf gdb clang)
			SDL_PC_NAMES=(sdl3 sdl3-image sdl3-ttf)
			SDL_PKGS=(sdl3 sdl3_image sdl3_ttf)
			;;
		debian)
			PKG_MANAGER="apt"
			INSTALL_CMD=(sudo apt-get install -y)
			TOOL_PKGS=(build-essential cmake ninja-build git pkg-config gdb clangd)
			SDL_PC_NAMES=(sdl3 SDL3_image SDL3_ttf)
			SDL_PKGS=(libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev)
			;;
		fedora)
			PKG_MANAGER="dnf"
			INSTALL_CMD=(sudo dnf install -y)
			TOOL_PKGS=(gcc gcc-c++ cmake ninja-build git pkgconf gdb clang clang-tools-extra)
			SDL_PC_NAMES=(sdl3 SDL3_image SDL3_ttf)
			SDL_PKGS=(SDL3-devel SDL3_image-devel SDL3_ttf-devel)
			;;
		suse)
			PKG_MANAGER="zypper"
			INSTALL_CMD=(sudo zypper install -y)
			TOOL_PKGS=(gcc gcc-c++ cmake ninja git pkg-config gdb clang)
			SDL_PC_NAMES=(sdl3 SDL3_image SDL3_ttf)
			SDL_PKGS=(libSDL3-devel libSDL3_image-devel libSDL3_ttf-devel)
			;;
		void)
			PKG_MANAGER="xbps"
			INSTALL_CMD=(sudo xbps-install -Sy)
			TOOL_PKGS=(gcc cmake ninja git pkg-config gdb)
			SDL_PC_NAMES=(sdl3 SDL3_image SDL3_ttf)
			SDL_PKGS=(SDL3-devel SDL3_image-devel SDL3_ttf-devel)
			;;
		alpine)
			PKG_MANAGER="apk"
			INSTALL_CMD=(sudo apk add --no-cache)
			TOOL_PKGS=(build-base cmake ninja git pkgconf gdb)
			SDL_PC_NAMES=(sdl3 sdl3_image sdl3_ttf)
			SDL_PKGS=(sdl3-dev sdl3_image-dev sdl3_ttf-dev)
			;;
		gentoo)
			PKG_MANAGER="emerge"
			INSTALL_CMD=(sudo emerge --ask=n)
			TOOL_PKGS=(dev-build/cmake dev-build/ninja dev-vcs/git dev-util/pkgconf dev-util/gdb)
			SDL_PC_NAMES=(sdl3 SDL3_image SDL3_ttf)
			SDL_PKGS=(media-libs/libsdl3 media-libs/sdl3-image media-libs/sdl3-ttf)
			log_warn "Gentoo: ensure USE=dev is set for SDL3 packages."
			;;
		*)
			log_error "Unsupported distribution: ${DISTRO_ID}"
			log_error "Supported families: Arch, Debian/Ubuntu, Fedora/RHEL,"
			log_error "                    openSUSE, Void, Alpine, Gentoo."
			exit 1
			;;
		esac
	}

	# Direct match first, then ID_LIKE fallback.
	case "$id" in
	arch | manjaro | endeavouros | garuda | artix) resolve_for arch ;;
	debian | ubuntu | linuxmint | pop | elementary | zorin | kali | raspbian) resolve_for debian ;;
	fedora | rhel | centos | almalinux | rocky | nobara) resolve_for fedora ;;
	opensuse* | sles) resolve_for suse ;;
	void) resolve_for void ;;
	alpine) resolve_for alpine ;;
	gentoo) resolve_for gentoo ;;
	*)
		if [[ "$like" == *arch* ]]; then
			resolve_for arch
		elif [[ "$like" == *debian* || "$like" == *ubuntu* ]]; then
			resolve_for debian
		elif [[ "$like" == *fedora* || "$like" == *rhel* ]]; then
			resolve_for fedora
		elif [[ "$like" == *suse* ]]; then
			resolve_for suse
		else
			resolve_for unknown
		fi
		;;
	esac
}

# ── Dependency detection ───────────────────────────────────────────────────────
#
# MISSING_PKGS accumulates package names that need to be installed.
# It is populated by add_if_missing() and consumed by install_missing().
#
MISSING_PKGS=()

# add_if_missing PKG CHECK_CMD [LABEL]
#
#   PKG        — the distro package name to install if the check fails.
#   CHECK_CMD  — a shell expression that succeeds (exit 0) when the
#                dependency is already present. Evaluated with eval so
#                compound commands ("pkg-config --exists foo") work.
#   LABEL      — optional human-readable name for log output (defaults to PKG).
#
# If CHECK_CMD succeeds the package is logged as already present and skipped.
# If it fails the package is added to MISSING_PKGS for later installation.
#
add_if_missing() {
	local pkg="$1"
	local check="$2"
	local label="${3:-$pkg}"

	if eval "$check" >/dev/null 2>&1; then
		log_ok "${label} — already installed"
	else
		log_warn "${label} — missing"
		MISSING_PKGS+=("$pkg")
	fi
}

# check_dependencies
#
# Runs add_if_missing() for every required dependency. Can be called before
# and after installation; the second call is used for final verification.
# Sets DEP_CHECK_OK=true if nothing is missing, false otherwise.
#
check_dependencies() {
	# Reset state so this function is safe to call more than once.
	MISSING_PKGS=()
	DEP_CHECK_OK=true

	# ── Build tools ───────────────────────────────────────────────────────────
	# Each tool is checked by probing the binary with command -v.
	# The package name comes from the distro-specific TOOL_PKGS array, keyed
	# by a positional index that matches the order tools are listed below.
	# Using named checks (rather than iterating TOOL_PKGS blindly) means we
	# can use the right probe for each dependency regardless of package name.

	add_if_missing "${TOOL_PKGS[0]}" "command -v c++ || command -v g++ || command -v clang++" "C++ compiler"
	add_if_missing "${TOOL_PKGS[1]}" "command -v cmake" "CMake"
	add_if_missing "${TOOL_PKGS[2]}" "command -v ninja" "Ninja"
	add_if_missing "${TOOL_PKGS[3]}" "command -v git" "Git"
	add_if_missing "${TOOL_PKGS[4]}" "command -v pkg-config" "pkg-config"

	# gdb and clangd are optional debug tools; still tracked so they get
	# installed on a fresh machine, but their absence is not fatal.
	if [[ ${#TOOL_PKGS[@]} -ge 6 ]]; then
		add_if_missing "${TOOL_PKGS[5]}" "command -v gdb" "gdb"
	fi
	if [[ ${#TOOL_PKGS[@]} -ge 7 ]]; then
		add_if_missing "${TOOL_PKGS[6]}" "command -v clangd || command -v clang" "clang/clangd"
	fi

	# ── SDL3 development headers ───────────────────────────────────────────────
	# Probed via pkg-config using the distro-specific module names from
	# SDL_PC_NAMES. These names mirror exactly what CMakeLists.txt queries,
	# so a passing check here guarantees cmake will find the libraries too.
	#
	# SDL_PC_NAMES[0] = sdl3 core   (e.g. "sdl3"     on Arch, "sdl3" elsewhere)
	# SDL_PC_NAMES[1] = sdl3-image  (e.g. "sdl3-image" / "SDL3_image")
	# SDL_PC_NAMES[2] = sdl3-ttf    (e.g. "sdl3-ttf"   / "SDL3_ttf")

	if command -v pkg-config >/dev/null 2>&1; then
		add_if_missing "${SDL_PKGS[0]}" "pkg-config --exists ${SDL_PC_NAMES[0]}" "SDL3 (${SDL_PC_NAMES[0]})"
		add_if_missing "${SDL_PKGS[1]}" "pkg-config --exists ${SDL_PC_NAMES[1]}" "SDL3_image (${SDL_PC_NAMES[1]})"
		add_if_missing "${SDL_PKGS[2]}" "pkg-config --exists ${SDL_PC_NAMES[2]}" "SDL3_ttf (${SDL_PC_NAMES[2]})"
	else
		# pkg-config itself is missing — SDL checks cannot run yet.
		# They will be re-evaluated after pkg-config is installed.
		log_warn "pkg-config not available — SDL3 headers will be checked after install"
		MISSING_PKGS+=("${SDL_PKGS[@]}")
	fi

	[[ ${#MISSING_PKGS[@]} -eq 0 ]] || DEP_CHECK_OK=false
}

# report_dependencies
#
# Prints a human-readable summary after check_dependencies() has run.
# Call this to show the user what was found before prompting to install.
#
report_dependencies() {
	if "$DEP_CHECK_OK"; then
		log_ok "All development dependencies are already installed."
	else
		log_warn "${#MISSING_PKGS[@]} package(s) need to be installed:"
		for pkg in "${MISSING_PKGS[@]}"; do
			log_info "  - ${pkg}"
		done
	fi
}

# ── Package index update ───────────────────────────────────────────────────────
# Only called when there are packages to install.
update_index() {
	case "$PKG_MANAGER" in
	pacman) sudo pacman -Sy ;;
	apt) sudo apt-get update -qq ;;
	dnf) sudo dnf check-update -q || true ;; # exits 100 when updates exist
	zypper) sudo zypper refresh ;;
	xbps) sudo xbps-install -S ;;
	apk) sudo apk update ;;
	emerge) : ;;
	esac
}

# ── Selective installation ─────────────────────────────────────────────────────
# Installs only the packages in MISSING_PKGS. If the list is empty the
# package manager is never invoked.
install_missing() {
	if ((${#MISSING_PKGS[@]} == 0)); then
		log_ok "Nothing to install — skipping package manager."
		return
	fi

	if ! confirm "Install ${#MISSING_PKGS[@]} missing package(s)?"; then
		log_info "Installation cancelled."
		exit 0
	fi

	log_section "Updating package index"
	update_index

	log_section "Installing packages"
	"${INSTALL_CMD[@]}" "${MISSING_PKGS[@]}"
}

# ── Post-install verification ──────────────────────────────────────────────────
# Re-runs check_dependencies() and prints the result. Intentionally reuses
# the same detection logic rather than duplicating version-printing code.
verify_build_deps() {
	log_section "Verifying build dependencies"
	check_dependencies

	# Print versions for everything that passed, warnings for anything that
	# still fails. Using the same add_if_missing infrastructure means this
	# is not a separate code path — it's an audit of the same checks.
	if "$DEP_CHECK_OK"; then
		# Print installed versions for the tools we care most about.
		for cmd in cmake ninja git pkg-config; do
			if command -v "$cmd" &>/dev/null; then
				log_ok "${cmd}: $("$cmd" --version 2>&1 | head -1)"
			fi
		done
		if command -v c++ &>/dev/null; then
			log_ok "c++: $(c++ --version 2>&1 | head -1)"
		fi
		if command -v pkg-config &>/dev/null; then
			for i in 0 1 2; do
				local name="${SDL_PC_NAMES[$i]}"
				if pkg-config --exists "$name" 2>/dev/null; then
					log_ok "${name}: $(pkg-config --modversion "$name")"
				fi
			done
		fi
		log_ok "All build dependencies satisfied."
	else
		log_warn "Some dependencies are still missing after installation:"
		for pkg in "${MISSING_PKGS[@]}"; do
			log_warn "  - ${pkg}"
		done
		log_warn "You may need to install them manually or check your repositories."
	fi
}

# ── Repo clone / locate ────────────────────────────────────────────────────────
ensure_repo() {
	if [[ -f CMakeLists.txt && -d src ]]; then
		log_info "Already in project directory."
		PROJECT_DIR="$PWD"
		return
	fi

	log_section "Repository"
	log_info "No project source found in current directory."
	log_info "Repo URL: ${REPO_URL}"

	if ! confirm "Clone the repository here?"; then
		log_info "Skipping clone. Make sure to run this from the project root."
		PROJECT_DIR="$PWD"
		return
	fi

	git clone --depth=1 "$REPO_URL" lxpaint
	PROJECT_DIR="$PWD/lxpaint"
	cd "$PROJECT_DIR"
	log_ok "Cloned to: ${PROJECT_DIR}"
}

# ── CMake configure + build ───────────────────────────────────────────────────
configure_and_build() {
	local build_type="${1:-Debug}"
	local cores
	cores=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

	log_section "Configuring (${build_type})"
	cmake -B "$BUILD_DIR" -G Ninja \
		-DCMAKE_BUILD_TYPE="$build_type" \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_COLOR_DIAGNOSTICS=ON

	# Symlink compile_commands.json to project root for clangd / IDEs.
	if [[ -f "${BUILD_DIR}/compile_commands.json" ]]; then
		ln -sf "${BUILD_DIR}/compile_commands.json" compile_commands.json
		log_ok "compile_commands.json → ${BUILD_DIR}/compile_commands.json"
	fi

	log_section "Building (${cores} cores)"
	cmake --build "$BUILD_DIR" -j "$cores"
}

# ── Main ──────────────────────────────────────────────────────────────────────
main() {
	log_section "LXPaint Developer Environment Installer"
	log_info "Sets up compiler, CMake, Ninja, SDL3 dev headers, and more."

	detect_distro
	resolve_packages

	# ── Dependency check ──────────────────────────────────────────────────────
	# Inspect what is already present before touching the package manager.
	# If everything is installed this is where the script stops — no sudo,
	# no network, no package index update.
	log_section "Checking dependencies"
	check_dependencies
	report_dependencies

	install_missing

	# ── Verify (only when something was installed) ────────────────────────────
	# Re-run the same checks so the user gets confirmation that installation
	# actually worked, and gets clear errors if something is still missing.
	if ! "$DEP_CHECK_OK"; then
		verify_build_deps
	fi

	if "$SKIP_BUILD"; then
		log_ok "Dependencies installed. Skipping build (--no-build)."
		exit 0
	fi

	ensure_repo

	log_section "Build"
	if confirm "Build the project now in Debug mode?"; then
		configure_and_build Debug
		log_ok "Build complete: ${PROJECT_DIR}/${BUILD_DIR}/"
		log_info "Run with: ./${BUILD_DIR}/lxpaint"
	else
		log_info "Skipped build. Run manually:"
		log_info "  cmake -B ${BUILD_DIR} -G Ninja -DCMAKE_BUILD_TYPE=Debug"
		log_info "  cmake --build ${BUILD_DIR} -j\$(nproc)"
	fi

	log_section "Done"
	log_ok "Developer environment ready."
	log_info "Useful commands:"
	log_info "  ./build.sh          — incremental build"
	log_info "  ./build.sh debug    — Debug build"
	log_info "  ./build.sh release  — Release build"
	log_info "  ./build.sh run      — Build and run"
	log_info "  ./build.sh clean    — Remove build directory"
}

main "$@"
