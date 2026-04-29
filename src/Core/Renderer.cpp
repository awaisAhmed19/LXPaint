#include "Renderer.h"

namespace Renderer {

void drawVerticalSpan(int x, int y, int thickness, uint32_t *pixels, int pitch,
                      int surfW, int surfH, uint32_t color, bool useXOR) {

  if (x < 0 || x >= surfW)
    return;

  int half = thickness / 2;
  int startY = std::max(0, y - half);
  int endY = std::min(surfH - 1, y + half);

  uint32_t xorColor = color & 0x00FFFFFF;

  for (int py = startY; py <= endY; py++) {
    uint32_t &px = pixels[py * pitch + x];
    if (useXOR)
      px ^= xorColor;
    else
      px = color;
  }
}

void drawHorizontalSpan(int x, int y, int thickness, uint32_t *pixels,
                        int pitch, int surfW, int surfH, uint32_t color,
                        bool useXOR) {

  if (y < 0 || y >= surfH)
    return;

  int half = thickness / 2;
  int startX = std::max(0, x - half);
  int endX = std::min(surfW - 1, x + half);

  uint32_t *row = pixels + y * pitch;
  uint32_t xorColor = color & 0x00FFFFFF;

  if (useXOR) {
    for (int px = startX; px <= endX; px++) {
      row[px] ^= xorColor;
    }
  } else {
    std::fill(row + startX, row + endX + 1, color);
  }
}

void dda(vec2<float> start, vec2<float> end, Canvas &canvas, uint32_t color,
         int brushSize, bool useXOR) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;
  int steps = std::abs(dx) > std::abs(dy) ? std::abs(dx) : std::abs(dy);
  if (steps == 0)
    return;

  if (!lockSurface(canvas.drawingSurface))
    return;

  float xInc = dx / (float)steps;
  float yInc = dy / (float)steps;
  float x = start.x, y = start.y;

  uint32_t *pixels = (uint32_t *)canvas.drawingSurface->pixels;
  int pitch = canvas.drawingSurface->pitch >> 2;
  int surfW = canvas.drawingSurface->w;
  int surfH = canvas.drawingSurface->h;

  for (int i = 0; i <= steps; i++) {
    int ix = (int)std::round(x);
    int iy = (int)std::round(y);

    // --- Optimized Brush Block ---
    for (int oy = -brushSize; oy <= brushSize; oy++) {
      int py = iy + oy;
      if (py < 0 || py >= surfH)
        continue;

      uint32_t *row = pixels + (py * pitch);

      int startX = std::max(0, ix - brushSize);
      int endX = std::min(surfW - 1, ix + brushSize);

      if (useXOR) {
        for (int px = startX; px <= endX; px++) {
          row[px] ^= (color & 0x00FFFFFF);
        }
      } else {
        std::fill(row + startX, row + endX + 1, color);
      }
    }
    x += xInc;
    y += yInc;
  }
  unlockSurface(canvas.drawingSurface);
}

void bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
               uint32_t color, int brushSize, bool useXOR) {
  if (!lockSurface(canvas.drawingSurface))
    return;

  int x1 = (int)start.x, y1 = (int)start.y;
  int x2 = (int)end.x, y2 = (int)end.y;

  int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
  int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
  int err = dx + dy;

  uint32_t *pixels = (uint32_t *)canvas.drawingSurface->pixels;
  int pitch = canvas.drawingSurface->pitch >> 2;
  int surfW = canvas.drawingSurface->w;
  int surfH = canvas.drawingSurface->h;

  bool steep = abs(y2 - y1) > abs(x2 - x1);

  while (true) {

    if (steep) {
      if (y1 >= 0 && y1 < surfH)
        drawHorizontalSpan(x1, y1, brushSize, pixels, pitch, surfW, surfH,
                           color, useXOR);

    } else {
      if (x1 >= 0 && x1 < surfW)
        drawVerticalSpan(x1, y1, brushSize, pixels, pitch, surfW, surfH, color,
                         useXOR);
    }
    if (x1 == x2 && y1 == y2)
      break;

    int e2 = 2 * err;

    if (e2 >= dy) {
      err += dy;
      x1 += sx;
    }
    if (e2 <= dx) {
      err += dx;
      y1 += sy;
    }
  }
  unlockSurface(canvas.drawingSurface);
}
} // namespace Renderer
