#include "ImageIO.h"

#include "Document/Canvas.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cctype>
#include <stb_image.h>
#include <stb_image_write.h>
namespace {

bool loadSurface(Canvas &canvas, SDL_Surface *loaded) {
  if (!loaded)
    return false;

  SDL_Surface *converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_ARGB8888);

  SDL_DestroySurface(loaded);

  if (!converted)
    return false;

  ResizePolicy policy;
  policy.anchor = ResizeAnchor::TOPLEFT;
  policy.fill = ResizeFill::TRANSPARENT;

  canvas.resize(converted->w, converted->h, policy);

  SDL_BlitSurface(converted, nullptr, canvas.getSurface(), nullptr);

  canvas.markDirty();

  SDL_DestroySurface(converted);

  return true;
}

} // namespace

bool ImageIO::save(const Canvas &canvas, const std::filesystem::path &path) {
  auto ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (ext == ".bmp")
    return saveBMP(canvas, path);

  if (ext == ".png")
    return savePNG(canvas, path);

  return false;
}
bool ImageIO::load(Canvas &canvas, const std::filesystem::path &path) {
  auto ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });

  if (ext == ".bmp")
    return loadBMP(canvas, path);

  if (ext == ".png")
    return loadPNG(canvas, path);

  if (ext == ".jpg")
    return loadJPG(canvas, path);

  if (ext == ".jpeg")
    return loadJPEG(canvas, path);

  if (ext == ".gif")
    return loadGIF(canvas, path);

  if (ext == ".tga")
    return loadTGA(canvas, path);

  if (ext == ".webp")
    return loadWEBP(canvas, path);

  if (ext == ".tif" || ext == ".tiff")
    return loadTIFF(canvas, path);

  return false;
}
bool ImageIO::saveBMP(const Canvas &canvas, const std::filesystem::path &path) {
  return SDL_SaveBMP(canvas.getSurface(), path.string().c_str());
}

bool ImageIO::savePNG(const Canvas &canvas, const std::filesystem::path &path) {
  SDL_Surface *s = canvas.getSurface();

  return stbi_write_png(path.string().c_str(), s->w, s->h, 4, s->pixels,
                        s->pitch) != 0;
}

bool ImageIO::loadPNG(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadBMP(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadJPG(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadJPEG(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadGIF(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadWEBP(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadTIFF(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}

bool ImageIO::loadTGA(Canvas &canvas, const std::filesystem::path &path) {
  return loadSurface(canvas, IMG_Load(path.string().c_str()));
}
