#!/usr/bin/env bash
# =============================================================================
# install.sh — LXPaint runtime installer (end users)
#
# Installs only the shared libraries needed to RUN LXPaint.
# Never installs compilers, headers, or build tools.
#
# Usage:
#   ./install.sh          # interactive
#   ./install.sh --yes    # non-interactive (accept all prompts)
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

die() {
	log_error "$*"
	exit 1
}

# ── Options ───────────────────────────────────────────────────────────────────
AUTO_YES=false
for arg in "$@"; do
	case "$arg" in
	--yes | -y) AUTO_YES=true ;;
	--help | -h)
		echo "Usage: $0 [--yes]"
		echo "  Installs LXPaint runtime dependencies only."
		exit 0
		;;
	*) die "Unknown argument: $arg" ;;
	esac
done

confirm() {
	# $1 = prompt message
	if "$AUTO_YES"; then return 0; fi
	read -rp "$1 [y/N] " ans
	[[ "$ans" =~ ^[Yy]$ ]]
}

# ── Sudo handling ─────────────────────────────────────────────────────────────
# In containers (Docker builds) we're already root and there is no `sudo`
# binary at all. NEED_SUDO is a plain boolean — every place that previously
# prefixed a command with "$SUDO" now branches into two fully-literal array
# assignments instead. This keeps every array element a real, static token,
# so shfmt/shellcheck-driven formatters have nothing to "fix" by re-quoting
# an empty variable.
NEED_SUDO=true
if [[ $EUID -eq 0 ]]; then
	NEED_SUDO=false
fi

# ── Distribution detection ────────────────────────────────────────────────────
detect_distro() {
	if [[ ! -f /etc/os-release ]]; then
		die "/etc/os-release not found. Cannot detect Linux distribution."
	fi

	# Source only the variables we need
	local ID="" ID_LIKE=""
	# shellcheck disable=SC1091
	eval "$(grep -E '^(ID|ID_LIKE)=' /etc/os-release | sed 's/"//')"

	DISTRO_ID="${ID,,}"        # lowercase
	DISTRO_LIKE="${ID_LIKE,,}" # may be empty

	log_info "Detected distribution: ${ID}"
	[[ -n "$DISTRO_LIKE" ]] && log_info "Distribution family: ${DISTRO_LIKE}"
}

# ── Package manager resolution ────────────────────────────────────────────────
#
# Resolves: PKG_MANAGER, INSTALL_CMD, and the list of runtime packages.
# SDL3/SDL3_image/SDL3_ttf package names differ by distro.
#
resolve_package_manager() {
	case "$DISTRO_ID" in
	arch | manjaro | endeavouros | garuda | artix)
		PKG_MANAGER="pacman"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo pacman -S --needed --noconfirm)
		else
			INSTALL_CMD=(pacman -S --needed --noconfirm)
		fi
		RUNTIME_PKGS=(sdl3 sdl3_image sdl3_ttf)
		;;
	*)
		# Fall through to ID_LIKE checks
		if [[ "$DISTRO_LIKE" == *arch* ]]; then
			PKG_MANAGER="pacman"
			if "$NEED_SUDO"; then
				INSTALL_CMD=(sudo pacman -S --needed --noconfirm)
			else
				INSTALL_CMD=(pacman -S --needed --noconfirm)
			fi
			RUNTIME_PKGS=(sdl3 sdl3_image sdl3_ttf)
			return
		fi
		;;
	esac

	[[ -n "${PKG_MANAGER:-}" ]] && return

	case "$DISTRO_ID" in
	debian | ubuntu | linuxmint | pop | elementary | zorin | kali | raspbian)
		PKG_MANAGER="apt"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo apt-get install -y)
		else
			INSTALL_CMD=(apt-get install -y)
		fi
		# SDL3 landed in Debian trixie / Ubuntu 24.04+
		# Older releases may require a PPA or manual build.
		RUNTIME_PKGS=(libsdl3-0 libsdl3-image-0 libsdl3-ttf-0)
		;;
	*)
		if [[ "$DISTRO_LIKE" == *debian* || "$DISTRO_LIKE" == *ubuntu* ]]; then
			PKG_MANAGER="apt"
			if "$NEED_SUDO"; then
				INSTALL_CMD=(sudo apt-get install -y)
			else
				INSTALL_CMD=(apt-get install -y)
			fi
			RUNTIME_PKGS=(libsdl3-0 libsdl3-image-0 libsdl3-ttf-0)
			return
		fi
		;;
	esac

	[[ -n "${PKG_MANAGER:-}" ]] && return

	case "$DISTRO_ID" in
	fedora | rhel | centos | almalinux | rocky | nobara)
		PKG_MANAGER="dnf"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo dnf install -y)
		else
			INSTALL_CMD=(dnf install -y)
		fi
		RUNTIME_PKGS=(SDL3 SDL3_image SDL3_ttf)
		;;
	*)
		if [[ "$DISTRO_LIKE" == *fedora* || "$DISTRO_LIKE" == *rhel* ]]; then
			PKG_MANAGER="dnf"
			if "$NEED_SUDO"; then
				INSTALL_CMD=(sudo dnf install -y)
			else
				INSTALL_CMD=(dnf install -y)
			fi
			RUNTIME_PKGS=(SDL3 SDL3_image SDL3_ttf)
			return
		fi
		;;
	esac

	[[ -n "${PKG_MANAGER:-}" ]] && return

	case "$DISTRO_ID" in
	opensuse* | sles)
		PKG_MANAGER="zypper"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo zypper install -y)
		else
			INSTALL_CMD=(zypper install -y)
		fi
		RUNTIME_PKGS=(libSDL3-0 libSDL3_image-0 libSDL3_ttf-0)
		;;
	*)
		if [[ "$DISTRO_LIKE" == *suse* ]]; then
			PKG_MANAGER="zypper"
			if "$NEED_SUDO"; then
				INSTALL_CMD=(sudo zypper install -y)
			else
				INSTALL_CMD=(zypper install -y)
			fi
			RUNTIME_PKGS=(libSDL3-0 libSDL3_image-0 libSDL3_ttf-0)
			return
		fi
		;;
	esac

	[[ -n "${PKG_MANAGER:-}" ]] && return

	case "$DISTRO_ID" in
	void)
		PKG_MANAGER="xbps"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo xbps-install -Sy)
		else
			INSTALL_CMD=(xbps-install -Sy)
		fi
		RUNTIME_PKGS=(SDL3 SDL3_image SDL3_ttf)
		;;
	alpine)
		PKG_MANAGER="apk"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo apk add --no-cache)
		else
			INSTALL_CMD=(apk add --no-cache)
		fi
		RUNTIME_PKGS=(sdl3 sdl3_image sdl3_ttf)
		;;
	gentoo)
		PKG_MANAGER="emerge"
		if "$NEED_SUDO"; then
			INSTALL_CMD=(sudo emerge --ask=n)
		else
			INSTALL_CMD=(emerge --ask=n)
		fi
		RUNTIME_PKGS=(media-libs/libsdl3 media-libs/sdl3-image media-libs/sdl3-ttf)
		log_warn "Gentoo support is best-effort. USE flags may need manual adjustment."
		;;
	*)
		log_error "Unsupported distribution: ${DISTRO_ID}"
		log_error "Supported: Arch/Manjaro/EndeavourOS, Debian/Ubuntu/Mint/Pop,"
		log_error "           Fedora/RHEL, openSUSE, Void, Alpine, Gentoo."
		log_error ""
		log_error "You can install the following manually:"
		log_error "  - SDL3 runtime library"
		log_error "  - SDL3_image runtime library"
		log_error "  - SDL3_ttf runtime library"
		exit 1
		;;
	esac
}

# ── Availability check ────────────────────────────────────────────────────────
check_already_installed() {
	# Use pkg-config as the ground truth — if the library is findable, it's
	# installed. This avoids false negatives from package manager state.
	local all_present=true
	local libs=(sdl3 SDL3_image SDL3_ttf)

	if command -v pkg-config &>/dev/null; then
		for lib in "${libs[@]}"; do
			if ! pkg-config --exists "$lib" 2>/dev/null; then
				all_present=false
				break
			fi
		done
	else
		all_present=false
	fi

	if "$all_present"; then
		log_ok "All runtime libraries are already installed."
		local sdl_ver
		sdl_ver=$(pkg-config --modversion sdl3 2>/dev/null || echo "unknown")
		log_info "SDL3 version: ${sdl_ver}"
		return 0
	fi
	return 1
}

# ── Update package index ───────────────────────────────────────────────────────
update_package_index() {
	if "$NEED_SUDO"; then
		case "$PKG_MANAGER" in
		pacman) sudo pacman -Sy ;;
		apt) sudo apt-get update -qq ;;
		dnf) sudo dnf check-update -q || true ;; # dnf returns 100 if updates exist
		zypper) sudo zypper refresh ;;
		xbps) sudo xbps-install -S ;;
		apk) sudo apk update ;;
		emerge) : ;; # emerge syncs automatically
		esac
	else
		case "$PKG_MANAGER" in
		pacman) pacman -Sy ;;
		apt) apt-get update -qq ;;
		dnf) dnf check-update -q || true ;; # dnf returns 100 if updates exist
		zypper) zypper refresh ;;
		xbps) xbps-install -S ;;
		apk) apk update ;;
		emerge) : ;; # emerge syncs automatically
		esac
	fi
}

# ── SDL3 availability warning ──────────────────────────────────────────────────
warn_if_sdl3_unavailable() {
	# Some older distro versions (Ubuntu <24.04, Debian <trixie) do not yet
	# ship SDL3 in their main repos. Detect this before attempting install.
	case "$PKG_MANAGER" in
	apt)
		if ! apt-cache show "${RUNTIME_PKGS[0]}" &>/dev/null; then
			log_warn "SDL3 runtime packages not found in apt repositories."
			log_warn "Your distribution version may be too old to ship SDL3."
			log_warn ""
			log_warn "Options:"
			log_warn "  1. Upgrade to Ubuntu 24.04+ or Debian trixie+"
			log_warn "  2. Build SDL3 from source (use dev-install.sh)"
			log_warn "  3. Add a third-party PPA if available"
			if ! confirm "Continue anyway (install may fail)?"; then
				exit 1
			fi
		fi
		;;
	dnf)
		if ! dnf list "${RUNTIME_PKGS[0]}" &>/dev/null; then
			log_warn "SDL3 not found in dnf repos. You may need RPM Fusion."
			if "$NEED_SUDO"; then
				log_warn "  sudo dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm"
			else
				log_warn "  dnf install https://download1.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm"
			fi
			if ! confirm "Continue anyway?"; then
				exit 1
			fi
		fi
		;;
	esac
}

# ── Main ──────────────────────────────────────────────────────────────────────
main() {
	log_section "LXPaint Runtime Installer"
	log_info "This installer adds only the shared libraries needed to run LXPaint."
	log_info "It will not install compilers or build tools."

	detect_distro
	resolve_package_manager

	log_section "Checking installed libraries"
	if check_already_installed; then
		exit 0
	fi

	log_section "Preparing to install"
	log_info "Package manager : ${PKG_MANAGER}"
	log_info "Packages        : ${RUNTIME_PKGS[*]}"

	if ! confirm "Proceed with installation?"; then
		log_info "Installation cancelled."
		exit 0
	fi

	log_section "Updating package index"
	update_package_index

	warn_if_sdl3_unavailable

	log_section "Installing runtime packages"
	"${INSTALL_CMD[@]}" "${RUNTIME_PKGS[@]}"

	log_section "Verifying installation"
	if command -v pkg-config &>/dev/null; then
		for lib in sdl3 SDL3_image SDL3_ttf; do
			if pkg-config --exists "$lib" 2>/dev/null; then
				log_ok "${lib}: $(pkg-config --modversion "$lib")"
			else
				log_warn "${lib}: not found via pkg-config (may still work)"
			fi
		done
	fi

	log_ok "Runtime installation complete."
	log_info "You can now run LXPaint."
}

main "$@"
