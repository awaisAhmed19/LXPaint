#include "Pencil.h"
#include "../commands/DrawCommand.h"
#include "../core/Logger.h"

static void _putpixel(SDL_Surface *surface, int x, int y, uint32_t color) {
  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return;

  uint32_t *pixels = (uint32_t *)surface->pixels;

  pixels[(y * (surface->pitch / 4)) + x] = color;
}

void Pencil::onMouseDown(vec2 pos, Canvas &canvas) {
  drawing = true;
  startpos = pos;

  // CRITICAL: Capture the canvas BEFORE we draw anything
  if (currentSnapshot)
    SDL_DestroySurface(currentSnapshot);
  currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);

  _putpixel(canvas.drawingSurface, startpos.x, startpos.y, color);
  Logger::log(LogLevel::DEBUG, "PENCIL STARTED DRAWING");
}

void Pencil::onMouseMove(vec2 pos, Canvas &canvas) {
  if (!drawing)
    return;

  vec2 currentPos = pos;
  // Connect last position to current position
  bresenham(startpos, currentPos, canvas, color, 1);
  Logger::log(LogLevel::DEBUG, "PENCIL DRAWING ");
  startpos = currentPos; // Update for the next frame
}

Command *Pencil::onMouseUp(vec2 pos, Canvas &canvas) {
  drawing = false;

  // Pass the captured "before" and the current "after"
  // DrawCommand will make its own permanent copies
  Command *cmd = new DrawCommand(currentSnapshot, canvas.drawingSurface);

  // Clean up the temporary snapshot used during this stroke
  SDL_DestroySurface(currentSnapshot);
  currentSnapshot = nullptr;

  Logger::log(LogLevel::DEBUG, "PENCIL DRAWING STOPS MOUSE UP");
  return cmd;
}

void Pencil::bresenham(vec2 start, vec2 end, Canvas &canvas, uint32_t color,
                       int brushSize = 2) {
  int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
  int dy = -abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
  int err = dx + dy;

  while (true) {
    for (int ox = -brushSize; ox <= brushSize; ox++) {
      for (int oy = -brushSize; oy <= brushSize; oy++) {
        _putpixel(canvas.drawingSurface, start.x + ox, start.y + oy, color);
      }
    }

    if (start.x == end.x && start.y == end.y)
      break;
    int e2 = 2 * err;
    if (e2 >= dy) {
      err += dy;
      start.x += sx;
    }
    if (e2 <= dx) {
      err += dx;
      start.y += sy;
    }
  }
}

void Pencil::dda(vec2 start, vec2 end, Canvas &canvas, uint32_t color) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;

  // Determine the number of steps needed
  int steps = std::abs(dx) > std::abs(dy) ? std::abs(dx) : std::abs(dy);

  float xInc = dx / (float)steps;
  float yInc = dy / (float)steps;

  float x = start.x;
  float y = start.y;

  for (int i = 0; i <= steps; i++) {
    // We round because we're drawing to a pixel grid
    _putpixel(canvas.drawingSurface, std::round(x), std::round(y), color);
    x += xInc;
    y += yInc;
  }
}
