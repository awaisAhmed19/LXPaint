#pragma once
#include "App/Globals.h"
#include "Systems/Assert.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <algorithm>
namespace Rasterizer {

inline uint32_t *getPixels(SDL_Surface *surface) {
  LX_ASSERT(surface != nullptr, "Surface null in getPixels");
  LX_ASSERT(surface->pixels != nullptr, "Surface pixels null");
  return static_cast<uint32_t *>(surface->pixels);
}

inline int getPitch(SDL_Surface *surface) {
  LX_ASSERT(surface != nullptr, "Surface null in getPitch");
  LX_ASSERT(surface->pitch > 0, "Invalid surface pitch");
  return surface->pitch >> 2;
}
void bresenham(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
               int brushSize, bool useXOR);

void dda(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
         int brushSize, bool useXOR);
void floodFill(SDL_Surface *surface, vec2 pos, uint32_t newcolor);
void drawEllipse_theta(SDL_Surface *surface, int x, int y, int w, int h,
                       uint32_t color);
void drawEllipse(SDL_Surface *surface, int xc, int yc, int rx, int ry,
                 uint32_t color);
void drawCircle(SDL_Surface *surface, int x_centre, int y_centre, int r,
                uint32_t color);
void drawPixel(SDL_Surface *surface, int x, int y, uint32_t color);

inline void drawHorizontalSpan(SDL_Surface *surface, int x, int y,
                               int thickness, uint32_t color, bool useXOR) {
  LX_ASSERT(surface != nullptr, "drawHorizontalSpan surface null");
  LX_ASSERT(thickness > 0, "Invalid brush thickness");
  if (y < 0 || y >= surface->h)
    return;

  const int half = thickness / 2;

  int startX = std::max(0, x - half);
  int endX = std::min(surface->w - 1, x + half);

  // Span completely outside canvas
  if (startX > endX)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  uint32_t *row = pixels + y * pitch;

  uint32_t xorColor = color & 0x00FFFFFF;

  if (useXOR) {
    for (int px = startX; px <= endX; ++px) {
      row[px] ^= xorColor;
    }
  } else {
    std::fill(row + startX, row + endX + 1, color);
  }
}
inline void drawVerticalSpan(SDL_Surface *surface, int x, int y, int thickness,
                             uint32_t color, bool useXOR) {
  LX_ASSERT(surface != nullptr, "drawVerticalSpan surface null");
  LX_ASSERT(thickness > 0, "Invalid brush thickness");

  if (x < 0 || x >= surface->w)
    return;

  const int half = thickness / 2;

  int startY = std::max(0, y - half);
  int endY = std::min(surface->h - 1, y + half);

  // Span completely outside canvas
  if (startY > endY)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  uint32_t xorColor = color & 0x00FFFFFF;

  for (int py = startY; py <= endY; ++py) {
    uint32_t &px = pixels[py * pitch + x];

    if (useXOR)
      px ^= xorColor;
    else
      px = color;
  }
}

void rectFill(SDL_Surface *surface, int minX, int minY, int maxX, int maxY,
              uint32_t color);

void rectFillWhite(SDL_Surface *surface, int minX, int minY, int maxX,
                   int maxY);

void drawRectStroke(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                    int brushSize);

void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  int brushSize, bool isWhiteFill);

void floodFill(SDL_Surface *surface, vec2 pos, uint32_t newcolor);

}; // namespace Rasterizer
