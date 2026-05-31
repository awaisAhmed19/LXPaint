#include <cmath>

#include "Rasterizer.h"
#include "Systems/Assert.h"
namespace Rasterizer {

static inline uint32_t *getPixels(SDL_Surface *surface) {
  LX_ASSERT(surface != nullptr, "Surface null in getPixels");
  LX_ASSERT(surface->pixels != nullptr, "Surface pixels null");
  return static_cast<uint32_t *>(surface->pixels);
}

static inline int getPitch(SDL_Surface *surface) {
  LX_ASSERT(surface != nullptr, "Surface null in getPitch");
  LX_ASSERT(surface->pitch > 0, "Invalid surface pitch");
  return surface->pitch >> 2;
}

void drawPixel(SDL_Surface *surface, int x, int y, uint32_t color) {
  LX_ASSERT(surface != nullptr, "drawPixel received null surface");
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  pixels[y * pitch + x] = color;
}

void drawVerticalSpan(SDL_Surface *surface, int x, int y, int thickness,
                      uint32_t color, bool useXOR) {
  LX_ASSERT(surface != nullptr, "drawVerticalSpan surface null");
  LX_ASSERT(thickness > 0, "Invalid brush thickness");
  if (x < 0 || x >= surface->w)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  int half = thickness / 2;

  int startY = std::max(0, y - half);
  int endY = std::min(surface->h - 1, y + half);

  uint32_t xorColor = color & 0x00FFFFFF;

  for (int py = startY; py <= endY; py++) {
    uint32_t &px = pixels[py * pitch + x];

    if (useXOR)
      px ^= xorColor;
    else
      px = color;
  }
}

void drawHorizontalSpan(SDL_Surface *surface, int x, int y, int thickness,
                        uint32_t color, bool useXOR) {
  LX_ASSERT(surface != nullptr, "drawHorizontalSpan surface null");
  LX_ASSERT(thickness > 0, "Invalid brush thickness");
  if (y < 0 || y >= surface->h)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  int half = thickness / 2;

  int startX = std::max(0, x - half);
  int endX = std::min(surface->w - 1, x + half);

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

void drawEllipse(SDL_Surface *surface, int xc, int yc, int rx, int ry,
                 uint32_t color) {
  float dx, dy, d1, d2;
  int x = 0;
  int y = ry;

  // Region 1
  d1 = (ry * ry) - (rx * rx * ry) + (0.25f * rx * rx);

  dx = 2 * ry * ry * x;
  dy = 2 * rx * rx * y;

  while (dx < dy) {
    drawPixel(surface, x + xc, y + yc, color);
    drawPixel(surface, -x + xc, y + yc, color);
    drawPixel(surface, x + xc, -y + yc, color);
    drawPixel(surface, -x + xc, -y + yc, color);

    if (d1 < 0) {
      x++;
      dx += 2 * ry * ry;
      d1 += dx + (ry * ry);
    } else {
      x++;
      y--;

      dx += 2 * ry * ry;
      dy -= 2 * rx * rx;

      d1 += dx - dy + (ry * ry);
    }
  }

  // Region 2
  d2 = ((ry * ry) * ((x + 0.5f) * (x + 0.5f))) +
       ((rx * rx) * ((y - 1) * (y - 1))) - (rx * rx * ry * ry);

  while (y >= 0) {
    drawPixel(surface, x + xc, y + yc, color);
    drawPixel(surface, -x + xc, y + yc, color);
    drawPixel(surface, x + xc, -y + yc, color);
    drawPixel(surface, -x + xc, -y + yc, color);

    if (d2 > 0) {
      y--;
      dy -= 2 * rx * rx;
      d2 += (rx * rx) - dy;
    } else {
      y--;
      x++;

      dx += 2 * ry * ry;
      dy -= 2 * rx * rx;

      d2 += dx - dy + (rx * rx);
    }
  }
}
void drawCircle(SDL_Surface *surface, int x_centre, int y_centre, int r,
                uint32_t color) {
  int x = r;
  int y = 0;

  int P = 1 - r;

  while (x >= y) {
    // Octant symmetry

    drawPixel(surface, x + x_centre, y + y_centre, color);
    drawPixel(surface, -x + x_centre, y + y_centre, color);
    drawPixel(surface, x + x_centre, -y + y_centre, color);
    drawPixel(surface, -x + x_centre, -y + y_centre, color);

    drawPixel(surface, y + x_centre, x + y_centre, color);
    drawPixel(surface, -y + x_centre, x + y_centre, color);
    drawPixel(surface, y + x_centre, -x + y_centre, color);
    drawPixel(surface, -y + x_centre, -x + y_centre, color);

    y++;

    if (P <= 0) {
      P = P + 2 * y + 1;
    } else {
      x--;
      P = P + 2 * y - 2 * x + 1;
    }
  }
}
void rectFill(SDL_Surface *surface, int minX, int minY, int maxX, int maxY,
              uint32_t color) {
  if (!lockSurface(surface))
    return;

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      drawPixel(surface, x, y, color);
    }
  }

  unlockSurface(surface);
}

void rectFillWhite(SDL_Surface *surface, int minX, int minY, int maxX,
                   int maxY) {
  if (!lockSurface(surface))
    return;

  constexpr uint32_t color = 0xFFFFFFFF;

  for (int y = minY + 1; y <= maxY - 1; y++) {
    for (int x = minX + 1; x <= maxX - 1; x++) {
      drawPixel(surface, x, y, color);
    }
  }

  unlockSurface(surface);
}

void drawRectStroke(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                    int brushSize) {
  int minX = std::min((int)a.x, (int)b.x);
  int minY = std::min((int)a.y, (int)b.y);

  int maxX = std::max((int)a.x, (int)b.x);
  int maxY = std::max((int)a.y, (int)b.y);

  bresenham({(float)minX, (float)minY}, {(float)maxX, (float)minY}, surface,
            color, brushSize, false);

  bresenham({(float)minX, (float)maxY}, {(float)maxX, (float)maxY}, surface,
            color, brushSize, false);

  bresenham({(float)minX, (float)minY}, {(float)minX, (float)maxY}, surface,
            color, brushSize, false);

  bresenham({(float)maxX, (float)minY}, {(float)maxX, (float)maxY}, surface,
            color, brushSize, false);
}

void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  bool isWhite) {
  drawRectStroke(surface, a, b, color, 1);
  if (isWhite) {
    rectFillWhite(surface, (int)a.x, (int)a.y, (int)b.x, (int)b.y);
  } else {
    rectFill(surface, (int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
  }
}

void dda(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
         int brushSize, bool useXOR) {
  LX_ASSERT(surface != nullptr, "DDA surface null");
  LX_ASSERT(brushSize > 0, "Invalid brush size");
  float dx = end.x - start.x;
  float dy = end.y - start.y;

  int steps = std::abs(dx) > std::abs(dy) ? std::abs(dx) : std::abs(dy);

  if (steps == 0)
    return;

  if (!lockSurface(surface))
    return;

  uint32_t *pixels = getPixels(surface);

  int pitch = getPitch(surface);

  float xInc = dx / (float)steps;
  float yInc = dy / (float)steps;

  float x = start.x;
  float y = start.y;

  for (int i = 0; i <= steps; i++) {
    int ix = (int)std::round(x);
    int iy = (int)std::round(y);

    for (int oy = -brushSize; oy <= brushSize; oy++) {
      int py = iy + oy;

      if (py < 0 || py >= surface->h)
        continue;

      uint32_t *row = pixels + py * pitch;

      int startX = std::max(0, ix - brushSize);
      int endX = std::min(surface->w - 1, ix + brushSize);

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

  unlockSurface(surface);
}

void bresenham(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
               int brushSize, bool useXOR) {
  LX_ASSERT(surface != nullptr, "Bresenham surface null");
  LX_ASSERT(brushSize > 0, "Invalid brush size");
  if (!lockSurface(surface))
    return;

  int x1 = (int)start.x;
  int y1 = (int)start.y;

  int x2 = (int)end.x;
  int y2 = (int)end.y;

  int dx = abs(x2 - x1);
  int sx = x1 < x2 ? 1 : -1;

  int dy = -abs(y2 - y1);
  int sy = y1 < y2 ? 1 : -1;

  int err = dx + dy;

  // FIXED: Steep-line transpose logic
  // When slope > 1 (steep line), we step in Y more than X
  // In this case, use horizontal spans (X-thickness) not vertical
  bool steep = abs(y2 - y1) > abs(x2 - x1);

  while (true) {
    // FIXED: Correct span selection based on steepness
    if (steep) {
      // Stepping primarily in Y: use horizontal span for brushSize in X
      drawHorizontalSpan(surface, x1, y1, brushSize, color, useXOR);
    } else {
      // Stepping primarily in X: use vertical span for brushSize in Y
      drawVerticalSpan(surface, x1, y1, brushSize, color, useXOR);
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

  unlockSurface(surface);
}

} // namespace Rasterizer
