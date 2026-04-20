#include "Line.h"
#include "../commands/DrawCommand.h"
#include "../core/Logger.h"
#include <algorithm>

static void _putpixelXOR(SDL_Surface *surface, int x, int y, uint32_t color) {
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;
  uint32_t *pixels = (uint32_t *)surface->pixels;
  int index = (y * (surface->pitch >> 2)) + x;
  pixels[index] ^= (color & 0x00FFFFFF);
}

static void _putpixel(SDL_Surface *surface, int x, int y, uint32_t color) {
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;
  uint32_t *pixels = (uint32_t *)surface->pixels;
  pixels[(y * (surface->pitch >> 2)) + x] = color;
}
// Add this to your Line.cpp or a helper
bool lockSurface(SDL_Surface *surface) {
  if (SDL_MUSTLOCK(surface)) {
    if (SDL_LockSurface(surface) < 0)
      return false;
  }
  return true;
}

void unlockSurface(SDL_Surface *surface) {
  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }
}
// THIS WAS MISSING OR INVALID IN YOUR SNIPPET
void Line::onMouseDown(vec2<float> pos, Canvas &canvas) {
  drawing = true;
  startpos = pos;
  lastMousePos = pos;

  if (currentSnapshot)
    SDL_DestroySurface(currentSnapshot);
  currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);

  Logger::log(LogLevel::DEBUG, "LINE TOOL: START");
}

void Line::onMouseMove(vec2<float> pos, Canvas &canvas) {
  if (!drawing || !currentSnapshot)
    return;

  // --- PHASE 1: RESTORATION ---
  switch (g_OptMode) {
  case OptimizationMode::BRUTE_FORCE:
    SDL_BlitSurface(currentSnapshot, NULL, canvas.drawingSurface, NULL);
    break;
  case OptimizationMode::DIRTY_RECT: {
    SDL_Rect dirty;
    dirty.x = (int)std::min(startpos.x, lastMousePos.x) - (brushSize + 2);
    dirty.y = (int)std::min(startpos.y, lastMousePos.y) - (brushSize + 2);
    dirty.w = (int)std::abs(startpos.x - lastMousePos.x) + (brushSize * 2 + 4);
    dirty.h = (int)std::abs(startpos.y - lastMousePos.y) + (brushSize * 2 + 4);
    SDL_BlitSurface(currentSnapshot, &dirty, canvas.drawingSurface, &dirty);
    break;
  }
  case OptimizationMode::OVERLAY:
    // Erase previous line using XOR
    bresenham(startpos, lastMousePos, canvas, color, brushSize, true);
    break;
  }

  // --- PHASE 2: THE RACE ---
  bool useXOR = (g_OptMode == OptimizationMode::OVERLAY);

  auto s1 = std::chrono::high_resolution_clock::now();
  bresenham(startpos, pos, canvas, color, brushSize, useXOR);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  dda(startpos, pos, canvas, color, brushSize, useXOR);
  auto e2 = std::chrono::high_resolution_clock::now();

  // --- PHASE 3: LOGGING ---
  float tBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float tDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

  Profiler::recordRaceStep({{"BRESENHAM", tBres, ImVec4(1, 0.5f, 0, 1)},
                            {"DDA", tDDA, ImVec4(0, 0.5f, 1, 1)}},
                           pos);

  lastMousePos = pos;
}

Command *Line::onMouseUp(vec2<float> pos, Canvas &canvas) {
  drawing = false;
  Command *cmd = new DrawCommand(currentSnapshot, canvas.drawingSurface);
  if (currentSnapshot) {
    SDL_DestroySurface(currentSnapshot);
    currentSnapshot = nullptr;
  }
  Profiler::commitRace(
      {{"BRESENHAM", ImVec4(1, 0.5f, 0, 1)}, {"DDA", ImVec4(0, 0.5f, 1, 1)}});
  return cmd;
}

void Line::bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
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
          if (useXOR)
            row[px] ^= (color & 0x00FFFFFF);
          else
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
void Line::dda(vec2<float> start, vec2<float> end, Canvas &canvas,
               uint32_t color, int brushSize, bool useXOR) {
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
          if (useXOR)
            row[px] ^= (color & 0x00FFFFFF);
          else
            row[px] = color;
        }
      }
    }
    x += xInc;
    y += yInc;
  }
  unlockSurface(canvas.drawingSurface);
}
