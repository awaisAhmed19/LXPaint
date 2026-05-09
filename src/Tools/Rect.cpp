#include "Rect.h"
#include "../Preview/PreviewSystem.h"
#include <algorithm>
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize);

  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

void Rect::onMouseDown(vec2 pos, SDL_Surface *surface) {
  drawing = true;
  Start = pos;
  Last = pos;

  Logger::log(LogLevel::DEBUG, "RECT TOOL: START");
}

void Rect::onMouseMove(vec2 pos, SDL_Surface *surface, PreviewSystem *ps) {
  if (!drawing)
    return;
  ps->clear();

  SDL_Rect newBound =
      computeRectBounds(Start, pos, brushSize, surface->w, surface->h);

  Last = pos;

  auto s1 = std::chrono::high_resolution_clock::now();
  Rasterizer::drawRect(ps->getSurface(), Start, pos, color, brushSize);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  auto e2 = std::chrono::high_resolution_clock::now();

  float tBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float tDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();
  ps->sync();
  Profiler::recordRaceStep({{"BRESENHAM", tBres, ImVec4(1, 0.5f, 0, 1)},
                            {"DDA", tDDA, ImVec4(0, 0.5f, 1, 1)}},
                           pos);
}

std::unique_ptr<Command> Rect::onMouseUp(vec2 pos, SDL_Surface *surface,
                                         PreviewSystem *ps) {
  drawing = false;
  ps->clear();

  SDL_Rect finalBound =
      computeRectBounds(Start, pos, brushSize, surface->w, surface->h);

  auto cmd = std::make_unique<DrawCommand>(surface, finalBound);
  Rasterizer::drawRect(surface, Start, pos, color, brushSize);
  cmd->captureAfter(surface);

  Profiler::commitRace(
      {{"BRESENHAM", ImVec4(1, 0.5f, 0, 1)}, {"DDA", ImVec4(0, 0.5f, 1, 1)}});

  return cmd;
}
