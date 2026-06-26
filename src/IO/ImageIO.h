#pragma once

#include <filesystem>

class Canvas;

namespace ImageIO {

// High-level API used by Editor.
bool save(const Canvas &canvas, const std::filesystem::path &path);
bool load(Canvas &canvas, const std::filesystem::path &path);

// Individual codecs (used internally, but public is fine for now).
bool saveBMP(const Canvas &canvas, const std::filesystem::path &path);
bool savePNG(const Canvas &canvas, const std::filesystem::path &path);

bool loadBMP(Canvas &canvas, const std::filesystem::path &path);
bool loadPNG(Canvas &canvas, const std::filesystem::path &path);
bool loadJPG(Canvas &canvas, const std::filesystem::path &path);
bool loadJPEG(Canvas &canvas, const std::filesystem::path &path);
bool loadTGA(Canvas &canvas, const std::filesystem::path &path);
bool loadGIF(Canvas &canvas, const std::filesystem::path &path);
bool loadWEBP(Canvas &canvas, const std::filesystem::path &path);
bool loadTIFF(Canvas &canvas, const std::filesystem::path &path);

} // namespace ImageIO
