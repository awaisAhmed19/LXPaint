#include "Pencil.h"
#include "../commands/DrawCommand.h"
#include "../core/Logger.h"

static void _putpixel(SDL_Surface *surface, int x, int y, uint32_t color) {
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;

  uint32_t *pixels = (uint32_t *)surface->pixels;

  pixels[(y * (surface->pitch >> 2)) + x] = color;
}

void Pencil::onMouseDown(vec2<float> pos, Canvas &canvas) {
  drawing = true;
  startpos = pos;

  // CRITICAL: Capture the canvas BEFORE we draw anything
  if (currentSnapshot)
    SDL_DestroySurface(currentSnapshot);
  currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);

  _putpixel(canvas.drawingSurface, startpos.x, startpos.y, color);
  Logger::log(LogLevel::DEBUG, "PENCIL STARTED DRAWING");
}

void Pencil::onMouseMove(vec2<float> pos, Canvas &canvas) {
  if (!drawing)
    return;
  auto s1 = std::chrono::high_resolution_clock::now();
  bresenham(startpos, pos, canvas, color, 1);
  auto e1 = std::chrono::high_resolution_clock::now();
  float usBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();

  auto s2 = std::chrono::high_resolution_clock::now();
  dda(startpos, pos, canvas, color, 1);
  auto e2 = std::chrono::high_resolution_clock::now();
  float usDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();
  Profiler::recordRaceStep(
      {
          {"BRESENHAM", usBres, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)}, // Orange
          {"DDA", usDDA, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}         // Blue
      },
      pos);

  startpos = pos;
}

Command *Pencil::onMouseUp(vec2<float> pos, Canvas &canvas) {
  drawing = false;
  Command *cmd = new DrawCommand(currentSnapshot, canvas.drawingSurface);
  if (currentSnapshot) {
    SDL_DestroySurface(currentSnapshot);
    currentSnapshot = nullptr;
  }

  Logger::log(LogLevel::DEBUG, "PENCIL DRAWING STOPS MOUSE UP");
  std::map<std::string, ImVec4> colorMap = {
      {"BRESENHAM", ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
      {"DDA", ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}};

  Profiler::commitRace(colorMap);
  return cmd;
}

void Pencil::bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
                       uint32_t color, int brushSize = 2) {
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

  while (true) {
    // --- Optimized Brush Block ---
    for (int oy = -brushSize; oy <= brushSize; oy++) {
      int py = y1 + oy;
      if (py < 0 || py >= surfH)
        continue;

      // Pre-calculate the start of the row
      uint32_t *row = pixels + (py * pitch);

      for (int ox = -brushSize; ox <= brushSize; ox++) {
        int px = x1 + ox;
        if (px >= 0 && px < surfW) {
          row[px] = color;
        }
      }
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

void Pencil::dda(vec2<float> start, vec2<float> end, Canvas &canvas,
                 uint32_t color, int brushSize) {
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

      for (int ox = -brushSize; ox <= brushSize; ox++) {
        int px = ix + ox;
        if (px >= 0 && px < surfW) {
          row[px] = color;
        }
      }
    }
    x += xInc;
    y += yInc;
  }
  unlockSurface(canvas.drawingSurface);
}
