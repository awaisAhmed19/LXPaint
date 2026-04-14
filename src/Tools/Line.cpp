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

// BRESENHAM & DDA IMPLEMENTATIONS... (your provided code is correct here)
void Line::bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
                     uint32_t color, int brushSize, bool useXOR) {
  int dx = abs((int)end.x - (int)start.x);
  int sx = start.x < end.x ? 1 : -1;
  int dy = -abs((int)end.y - (int)start.y);
  int sy = start.y < end.y ? 1 : -1;
  int err = dx + dy;

  int curX = (int)start.x;
  int curY = (int)start.y;

  while (true) {
    // Draw brush square
    for (int ox = -brushSize; ox <= brushSize; ox++) {
      for (int oy = -brushSize; oy <= brushSize; oy++) {
        if (useXOR)
          _putpixelXOR(canvas.drawingSurface, curX + ox, curY + oy, color);
        else
          _putpixel(canvas.drawingSurface, curX + ox, curY + oy, color);
      }
    }

    if (curX == (int)end.x && curY == (int)end.y)
      break;

    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      curX += sx;
    }
    if (e2 <= dx) {
      err += dx;
      curY += sy;
    }
  }
}

void Line::dda(vec2<float> start, vec2<float> end, Canvas &canvas,
               uint32_t color, int brushSize, bool useXOR) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;

  int steps = std::abs(dx) > std::abs(dy) ? std::abs(dx) : std::abs(dy);

  if (steps == 0) {
    for (int ox = -brushSize; ox <= brushSize; ox++) {
      for (int oy = -brushSize; oy <= brushSize; oy++) {
        if (useXOR)
          _putpixelXOR(canvas.drawingSurface, (int)start.x + ox,
                       (int)start.y + oy, color);
        else
          _putpixel(canvas.drawingSurface, (int)start.x + ox, (int)start.y + oy,
                    color);
      }
    }
    return;
  }

  float xInc = dx / (float)steps;
  float yInc = dy / (float)steps;

  float x = start.x;
  float y = start.y;

  for (int i = 0; i <= steps; i++) {
    // CRITICAL OPTIMIZATION: Round once per step, not per brush pixel
    int ix = (int)std::round(x);
    int iy = (int)std::round(y);

    for (int ox = -brushSize; ox <= brushSize; ox++) {
      for (int oy = -brushSize; oy <= brushSize; oy++) {
        if (useXOR)
          _putpixelXOR(canvas.drawingSurface, ix + ox, iy + oy, color);
        else
          _putpixel(canvas.drawingSurface, ix + ox, iy + oy, color);
      }
    }

    x += xInc;
    y += yInc;
  }
}
