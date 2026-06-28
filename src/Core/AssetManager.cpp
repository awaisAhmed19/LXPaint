#include "AssetManager.h"
#include "Systems/Logger.h"
#include <SDL3/SDL.h>

#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace AssetManager {

namespace {

std::filesystem::path s_root;
bool s_initialized = false;

// A "valid" assets root contains a "fonts" subdirectory.
// This distinguishes the real tree from a stray directory that happens to be
// named "assets".
bool isValidRoot(const std::filesystem::path &candidate) {
  std::error_code ec;
  return std::filesystem::is_directory(candidate, ec) &&
         std::filesystem::is_directory(candidate / "fonts", ec);
}

// Canonicalize without throwing; returns empty path on error.
std::filesystem::path tryCanonicalize(const std::filesystem::path &p) {
  std::error_code ec;
  auto result = std::filesystem::weakly_canonical(p, ec);
  return ec ? std::filesystem::path{} : result;
}

std::filesystem::path findRoot() {
  std::vector<std::filesystem::path> candidates;

  const char *sdlBase = SDL_GetBasePath();
  if (sdlBase) {
    std::filesystem::path binDir(sdlBase);
    candidates.push_back(binDir / "assets");
    candidates.push_back(binDir / ".." / "assets");
    candidates.push_back(binDir / ".." / ".." / "assets");
  }

  try {
    const auto cwd = std::filesystem::current_path();
    candidates.push_back(cwd / "assets");
    candidates.push_back(cwd / ".." / "assets");
  } catch (const std::filesystem::filesystem_error &) {
    // current_path() can fail in sandboxed environments; just skip.
  }

#ifdef LX_ASSET_DIR
  candidates.push_back(std::filesystem::path(LX_ASSET_DIR));
#endif

  for (const auto &raw : candidates) {
    auto canonical = tryCanonicalize(raw);
    if (!canonical.empty() && isValidRoot(canonical)) {
      return canonical;
    }
  }

  return {};
}

} // namespace

void initialize() {
  if (s_initialized)
    return;

  s_root = findRoot();
  s_initialized = true;

  if (s_root.empty()) {
    Logger::err("AssetManager: could not locate assets directory.\n"
                "  Searched: <exe>/assets, <exe>/../assets, "
                "<exe>/../../assets, <cwd>/assets, <cwd>/../assets"
#ifdef LX_ASSET_DIR
                ", " LX_ASSET_DIR
#endif
    );
  } else {
    Logger::debug(std::format("AssetManager: root = {}", s_root.string()));
  }
}

std::filesystem::path root() {
  if (!s_initialized)
    initialize();
  return s_root;
}

std::filesystem::path asset(std::string_view relative) {
  if (!s_initialized)
    initialize();

  const std::filesystem::path p = s_root / relative;

  std::error_code ec;
  if (!std::filesystem::exists(p, ec)) {
    // Build a human-readable list of the locations that were tried so the
    // developer can immediately see where to put the missing file.
    Logger::err(std::format("Asset not found: {}\n"
                            "  full path searched: {}",
                            std::string(relative), p.string()));
  }

  return p;
}

std::filesystem::path font(std::string_view filename) {
  return asset((std::filesystem::path("fonts") / filename).string());
}

std::filesystem::path toolIcon(std::string_view filename) {
  return asset((std::filesystem::path("tools_icons") / filename).string());
}

std::filesystem::path cursor(std::string_view filename) {
  return asset((std::filesystem::path("cursors") / filename).string());
}

} // namespace AssetManager
