#include "Rasterizer.h"
#include "Editor/ToolSettings.h"
#include "Systems/Assert.h"
#include <SDL3/SDL_surface.h>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#define TAU 6.2831853
namespace Rasterizer {

static constexpr int BEZIER_STEPS = 100; // line segments to approximate curve
static constexpr int MARKER_HALF = 3; // half-size of square handle markers (px)
static constexpr int BOUNDS_PAD = 4;  // extra safety margin on affected rect
static constexpr int DASH_ON = 5;     // guide-line dash length (px)
static constexpr int DASH_OFF = 4;    // guide-line gap length (px)

// Colours for handle markers (ARGB)
static constexpr uint32_t COLOR_GUIDE = 0xFF808080;  // grey dashed guide lines
static constexpr uint32_t COLOR_ANCHOR = 0xFFFFFFFF; // white  endpoint handles
static constexpr uint32_t COLOR_CP1 = 0xFF0000FF;    // blue   CP1 handle
static constexpr uint32_t COLOR_CP2 = 0xFFFF0000;    // red    CP2 handle
static constexpr uint32_t COLOR_BORDER = 0xFF000000; // black  handle border

inline float randomFloat(float min, float max) {
  static thread_local std::mt19937 rng{std::random_device{}()};
  std::uniform_real_distribution<float> dist(min, max);
  return dist(rng);
}
void drawPixel(SDL_Surface *surface, int x, int y, uint32_t color) {
  LX_ASSERT(surface != nullptr, "drawPixel received null surface");
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;

  uint32_t *pixels = getPixels(surface);
  int pitch = getPitch(surface);

  pixels[y * pitch + x] = color;
}

inline void drawPolygon(SDL_Surface *surface, const std::vector<vec2> points,
                        uint32_t color) {
  if (points.size() < 2)
    return;
  for (int p = 0; p < (int)points.size() - 1; ++p) {
    bresenham(points[p], points[p + 1], surface, color, 1, false);
  }
  bresenham(points.back(), points.front(), surface, color, 1, false);
}
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize - 4);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize - 4);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize + 4);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize + 4);
  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

inline void drawPolygonOpaque(SDL_Surface *surface,
                              const std::vector<vec2> &points, uint32_t color) {
  if (!surface || points.size() < 2)
    return;

  for (size_t i = 0; i < points.size(); ++i) {
    bresenham(points[i], points[(i + 1) % points.size()], surface, color, 4,
              false);
  }
  uint32_t white = 0xFFFFFFFF;
  fillPolygon(surface, points, white);
}
inline void drawPolygonFill(SDL_Surface *surface,
                            const std::vector<vec2> &points, uint32_t color) {
  if (!surface || points.size() < 2)
    return;

  for (size_t i = 0; i < points.size(); ++i) {
    bresenham(points[i], points[(i + 1) % points.size()], surface, color, 4,
              false);
  }

  fillPolygon(surface, points, color);
}
static vec2 evalCubicBezier(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
  const float u = 1.0f - t;
  const float uu = u * u;
  const float uuu = uu * u;
  const float tt = t * t;
  const float ttt = tt * t;
  return {uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
          uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y +
              ttt * p3.y};
}

static void drawBezier(SDL_Surface *surf, vec2 p0, vec2 p1, vec2 p2, vec2 p3,
                       uint32_t color, int lw) {
  vec2 prev = p0;

  for (int i = 1; i <= BEZIER_STEPS; ++i) {
    float t = float(i) / BEZIER_STEPS;

    vec2 pt = evalCubicBezier(p0, p1, p2, p3, t);

    Rasterizer::bresenham(prev, pt, surf, color, lw, false);

    prev = pt;
  }
}
void drawEllipse(SDL_Surface *surface, int xc, int yc, int rx, int ry,
                 uint32_t color) {
  rx = std::abs(rx);
  ry = std::abs(ry);
  double dx, dy, d1, d2;
  int x = 0;
  int y = ry;

  auto plot4 = [&](int px, int py) {
    drawPixel(surface, xc + px, yc + py, color);
    drawPixel(surface, xc - px, yc + py, color);
    drawPixel(surface, xc + px, yc - py, color);
    drawPixel(surface, xc - px, yc - py, color);
  };

  d1 = (ry * ry) - (rx * rx * ry) + (0.25 * rx * rx);

  dx = 2 * ry * ry * x;
  dy = 2 * rx * rx * y;

  while (dx <= dy) {
    plot4(x, y);

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

  d2 = ((ry * ry) * ((x + 0.5) * (x + 0.5))) +
       ((rx * rx) * ((y - 1) * (y - 1))) - ((rx * rx) * (ry * ry));

  while (y >= 0) {
    plot4(x, y);

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

void appendBezierPoints(std::vector<vec2> &points, vec2 p0, vec2 p1, vec2 p2,
                        vec2 p3, int segments = 12) {
  int start = points.empty() ? 0 : 1;

  for (int i = start; i <= segments; ++i) {
    float t = (float)i / segments;
    float u = 1.0f - t;

    vec2 p = p0 * (u * u * u) + p1 * (3.0f * u * u * t) +
             p2 * (3.0f * u * t * t) + p3 * (t * t * t);

    points.push_back(p);
  }
}

void drawRoundedRect(SDL_Surface *surf, vec2 start, vec2 end, uint32_t color,
                     int lw, std::vector<vec2> &points) {
  const float left = std::min(start.x, end.x);
  const float right = std::max(start.x, end.x);
  const float top = std::min(start.y, end.y);
  const float bottom = std::max(start.y, end.y);

  const float width = right - left;
  const float height = bottom - top;
  constexpr float cornerRadius = 12.0f;

  const float r = std::min({cornerRadius, width * 0.5f, height * 0.5f});
  points.clear();
  // Cubic Bezier approximation constant for a quarter circle
  constexpr float K = 0.552284749831f;
  const float o = r * K;

  // Top edge
  points.push_back({left + r, top});
  points.push_back({right - r, top});

  // Top-right corner
  appendBezierPoints(points, {right - r, top}, {right - r + o, top},
                     {right, top + r - o}, {right, top + r});

  // Right edge
  points.push_back({right, bottom - r});

  // Bottom-right corner
  appendBezierPoints(points, {right, bottom - r}, {right, bottom - r + o},
                     {right - r + o, bottom}, {right - r, bottom});

  // Bottom edge
  points.push_back({left + r, bottom});

  // Bottom-left corner
  appendBezierPoints(points, {left + r, bottom}, {left + r - o, bottom},
                     {left, bottom - r + o}, {left, bottom - r});

  // Left edge
  points.push_back({left, top + r});

  // Top-left corner
  appendBezierPoints(points, {left, top + r}, {left, top + r - o},
                     {left + r - o, top}, {left + r, top});
  // ───────────── Straight edges ─────────────

  Rasterizer::bresenham({left + r, top}, {right - r, top}, surf, color, lw,
                        false);
  Rasterizer::bresenham({right, top + r}, {right, bottom - r}, surf, color, lw,
                        false);
  Rasterizer::bresenham({right - r, bottom}, {left + r, bottom}, surf, color,
                        lw, false);
  Rasterizer::bresenham({left, bottom - r}, {left, top + r}, surf, color, lw,
                        false);

  // ───────────── Top Left ─────────────

  drawBezier(surf, {left + r, top}, {left + r - o, top}, {left, top + r - o},
             {left, top + r}, color, lw);

  // ───────────── Top Right ─────────────

  drawBezier(surf, {right - r, top}, {right - r + o, top}, {right, top + r - o},
             {right, top + r}, color, lw);

  // ───────────── Bottom Right ─────────────

  drawBezier(surf, {right, bottom - r}, {right, bottom - r + o},
             {right - r + o, bottom}, {right - r, bottom}, color, lw);

  // ───────────── Bottom Left ─────────────

  drawBezier(surf, {left + r, bottom}, {left + r - o, bottom},
             {left, bottom - r + o}, {left, bottom - r}, color, lw);
}

void spray(SDL_Surface *surface, vec2 center, uint32_t color, float radius,
           int density) {
  for (int i = 0; i < density; ++i) {
    float angle = randomFloat(0.f, 2.f * std::numbers::pi_v<float>);
    float dist = std::sqrt(randomFloat(0.f, 1.f)) * radius;

    int x = static_cast<int>(center.x + std::cos(angle) * dist);
    int y = static_cast<int>(center.y + std::sin(angle) * dist);

    drawPixel(surface, x, y, color);
  }
}

void sprayStroke(SDL_Surface *surface, vec2 start, vec2 end, uint32_t color,
                 float radius, int density) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;

  float dist = std::hypot(dx, dy);

  int steps = std::max(1, static_cast<int>(dist));

  for (int i = 0; i <= steps; ++i) {
    float t = static_cast<float>(i) / steps;

    vec2 p{
        std::lerp(start.x, end.x, t),
        std::lerp(start.y, end.y, t),
    };

    spray(surface, p, color, radius, density);
  }
}

void stampBrush(SDL_Surface *surface, vec2 pos, uint32_t color,
                BrushShape shape, int size) {
  LX_ASSERT(surface != nullptr, "stampBrush surface null");

  const int cx = (int)pos.x;
  const int cy = (int)pos.y;

  // size is 1, 2, or 4 — half-extent in pixels from center.
  const int half = std::max(0, size - 1);

  switch (shape) {

  case BrushShape::Round: {
    // Reuse drawCircle for size>1; size==1 is a single pixel, matching the
    // classic MS Paint round brush at its smallest setting.
    if (size <= 1) {
      drawPixel(surface, cx, cy, color);
    } else {
      // drawCircle only plots the outline; fill the disc by sweeping
      // horizontal spans per row within radius, matching round-brush look.
      int r = half;
      for (int dy = -r; dy <= r; ++dy) {
        int dx = (int)std::round(std::sqrt((double)(r * r - dy * dy)));
        drawHorizontalSpan(surface, cx, cy + dy, dx * 2 + 1, color, false);
      }
    }
    break;
  }

  case BrushShape::Square: {
    for (int dy = -half; dy <= half; ++dy) {
      drawHorizontalSpan(surface, cx, cy + dy, half * 2 + 1, color, false);
    }
    break;
  }

  case BrushShape::ForwardSlash: {
    // "/" pattern: bottom-left to top-right diagonal footprint.
    for (int d = -half; d <= half; ++d) {
      drawPixel(surface, cx + d, cy - d, color);
    }
    break;
  }

  case BrushShape::BackSlash: {
    // "\" pattern: top-left to bottom-right diagonal footprint.
    for (int d = -half; d <= half; ++d) {
      drawPixel(surface, cx + d, cy + d, color);
    }
    break;
  }
  }
}

void brushStroke(SDL_Surface *surface, vec2 start, vec2 end, uint32_t color,
                 BrushShape shape, int size) {
  LX_ASSERT(surface != nullptr, "brushStroke surface null");

  float dx = end.x - start.x;
  float dy = end.y - start.y;

  float dist = std::hypot(dx, dy);

  // Same step-per-pixel interpolation style as sprayStroke, so the brush
  // never gaps between mouse-move samples regardless of movement speed.
  int steps = std::max(1, (int)dist);

  for (int i = 0; i <= steps; ++i) {
    float t = (float)i / (float)steps;

    vec2 p{
        std::lerp(start.x, end.x, t),
        std::lerp(start.y, end.y, t),
    };

    stampBrush(surface, p, color, shape, size);
  }
}

void fillPolygon(SDL_Surface *surface, const std::vector<vec2> &points,
                 uint32_t color) {
  LX_ASSERT(surface != nullptr, "fillPolygon surface null");

  if (points.size() < 3)
    return;

  int minY = surface->h, maxY = 0;
  for (const auto &p : points) {
    minY = std::min(minY, (int)std::floor(p.y));
    maxY = std::max(maxY, (int)std::ceil(p.y));
  }
  minY = std::max(0, minY);
  maxY = std::min(surface->h - 1, maxY);

  // Even-odd scanline fill — same rule Lasso::pointInPolygon already uses
  // for its selection mask, applied here as horizontal span fills instead
  // of a per-pixel point test for efficiency.
  size_t n = points.size();

  for (int y = minY; y <= maxY; ++y) {
    std::vector<float> intersections;
    float fy = (float)y + 0.5f;

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
      const vec2 &a = points[i];
      const vec2 &b = points[j];

      if (a.y == b.y)
        continue;

      if ((fy >= a.y && fy < b.y) || (fy >= b.y && fy < a.y)) {
        float x = a.x + (fy - a.y) * (b.x - a.x) / (b.y - a.y);
        intersections.push_back(x);
      }
    }

    std::sort(intersections.begin(), intersections.end());

    for (size_t k = 0; k + 1 < intersections.size(); k += 2) {
      int xStart = (int)std::round(intersections[k]);
      int xEnd = (int)std::round(intersections[k + 1]);
      xStart = std::max(0, xStart);
      xEnd = std::min(surface->w - 1, xEnd);

      if (xStart > xEnd)
        continue;

      // NOTE: deliberately not reusing drawHorizontalSpan here — that
      // helper is center+thickness shaped (half = thickness/2, integer
      // division), which drifts by a pixel on even-width spans when forced
      // through a start/end range via a synthesized center. Filling the
      // explicit [xStart, xEnd] range directly avoids that off-by-one.
      uint32_t *pixels = getPixels(surface);
      int pitch = getPitch(surface);
      uint32_t *row = pixels + y * pitch;
      std::fill(row + xStart, row + xEnd + 1, color);
    }
  }
}

void drawEllipse_theta(SDL_Surface *surface, int x, int y, int w, int h,
                       uint32_t color, ToolSettings::FillMode fillmode) {
  const int cx = x;
  const int cy = y;

  const float step = 0.05f;
  std::vector<vec2> points{};
  for (float theta = 0.0f; theta < TAU; theta += step) {
    float newX = (cx + std::cos(theta) * w);
    float newY = (cy + std::sin(theta) * h);
    points.push_back({newX, newY});
  }
  switch (fillmode) {
  case ToolSettings::FillMode::Outline:
    drawPolygon(surface, points, color);
    break;
  case ToolSettings::FillMode::Fill:
    drawPolygonOpaque(surface, points, color);
    break;
  case ToolSettings::FillMode::Opaque:
    drawPolygonFill(surface, points, color);
    break;
  }
}

/*Function draw_ellipse(ctx, x, y, w, h, stroke, fill) {
        const center_x = x + w / 2;
        const center_y = y + h / 2;

        if (aliasing) {
                const points = [];
                const step = 0.05;
                for (let theta = 0; theta < TAU; theta += step) {
                        points.push({
                                x: center_x + Math.cos(theta) * w / 2,
                                y: center_y + Math.sin(theta) * h / 2,
                        });
                }
                draw_polygon(ctx, points, stroke, fill);
        } else {
                ctx.beginPath();
                ctx.ellipse(center_x, center_y, Math.abs(w / 2), Math.abs(h /
2), 0, 0, TAU, false); ctx.stroke(); ctx.fill();
        }
}
 */

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
  int nminX = std::min(minX, maxX);
  int nminY = std::min(minY, maxY);

  int nmaxX = std::max(minX, maxX);
  int nmaxY = std::max(minY, maxY);
  if (!lockSurface(surface))
    return;

  for (int y = nminY; y <= nmaxY; y++) {
    for (int x = nminX; x <= nmaxX; x++) {
      drawPixel(surface, x, y, color);
    }
  }

  unlockSurface(surface);
}

void rectFillWhite(SDL_Surface *surface, int minX, int minY, int maxX,
                   int maxY) {
  int nminX = std::min(minX, maxX);
  int nminY = std::min(minY, maxY);

  int nmaxX = std::max(minX, maxX);
  int nmaxY = std::max(minY, maxY);
  if (!lockSurface(surface))
    return;

  constexpr uint32_t color = 0xFFFFFFFF;

  for (int y = nminY; y <= nmaxY; y++) {
    for (int x = nminX; x <= nmaxX; x++) {
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
                  int brushSize, bool isWhite) {
  drawRectStroke(surface, a, b, color, brushSize);
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
  int dy = abs(y2 - y1);

  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;

  int err = dx - dy;

  bool steep = abs(y2 - y1) > abs(x2 - x1);

  while (true) {
    if (steep) {
      drawHorizontalSpan(surface, x1, y1, brushSize, color, useXOR);
    } else {
      drawVerticalSpan(surface, x1, y1, brushSize, color, useXOR);
    }

    if (x1 == x2 && y1 == y2)
      break;

    int e2 = 2 * err;

    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }

    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }

  unlockSurface(surface);
}
void floodFillHelper(uint32_t *pixels, int pitch, int startX, int startY,
                     uint32_t oldColor, uint32_t newColor, int w, int h) {
  struct Point {
    int x;
    int y;
  };

  std::vector<Point> stack;
  stack.push_back({startX, startY});

  while (!stack.empty()) {
    Point p = stack.back();
    stack.pop_back();

    int x = p.x;
    int y = p.y;

    if (x < 0 || x >= w || y < 0 || y >= h) {
      continue;
    }

    uint32_t &pixel = pixels[y * pitch + x];

    if (pixel != oldColor) {
      continue;
    }

    pixel = newColor;

    stack.push_back({x - 1, y});
    stack.push_back({x + 1, y});
    stack.push_back({x, y - 1});
    stack.push_back({x, y + 1});
  }
}
void floodFill(SDL_Surface *surface, vec2 pos, uint32_t newColor) {
  uint32_t *pixels = getPixels(surface);

  int pitch = getPitch(surface);
  int w = surface->w;
  int h = surface->h;

  int x = (int)pos.x;
  int y = (int)pos.y;

  if (x < 0 || x >= w || y < 0 || y >= h) {
    return;
  }

  uint32_t oldColor = pixels[y * pitch + x];

  if (oldColor == newColor) {
    return;
  }

  floodFillHelper(pixels, pitch, x, y, oldColor, newColor, w, h);
}
} // namespace Rasterizer
