#pragma once

#include <filesystem>
#include <string_view>

// ─────────────────────────────────────────────────────────────────────────────
//  AssetManager
//
//  Single source of truth for locating and loading application assets.
//
//  Call initialize() once during Application startup (before any asset is
//  loaded).  Every subsequent call to asset(), font(), toolIcon(), cursor()
//  returns an absolute, verified path — or logs a clear error and returns a
//  best-effort path if the file is missing.
//
//  No other module should ever construct asset paths manually.
//
//  Search order (stops at the first valid root):
//    1. <executable directory>/assets/
//    2. <executable directory>/../assets/
//    3. <executable directory>/../../assets/
//    4. <cwd>/assets/
//    5. <cwd>/../assets/
//    6. LX_ASSET_DIR  (compile-time CMake definition, final fallback)
//
//  A "valid root" is a directory that contains a known sub-directory
//  ("fonts"), proving it is the real assets tree and not a stray folder.
// ─────────────────────────────────────────────────────────────────────────────

namespace AssetManager {

void initialize();
std::filesystem::path root();

// ── Generic resolver ──────────────────────────────────────────────────────

// Resolves any relative path under the asset root.
// Logs an error if the resulting path does not exist on disk.
//
// Example:
//   AssetManager::asset("fonts/NotoSans-Regular.ttf")
//   AssetManager::asset("tools_icons/07_pencil.png")
std::filesystem::path asset(std::string_view relative);

// ── Typed helpers ─────────────────────────────────────────────────────────

std::filesystem::path font(std::string_view filename);
std::filesystem::path toolIcon(std::string_view filename);
std::filesystem::path cursor(std::string_view filename);

} // namespace AssetManager
