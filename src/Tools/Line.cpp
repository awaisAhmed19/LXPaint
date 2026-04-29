#include "Line.h"
#include <algorithm>
static SDL_Rect computeLineBounds(vec2<float> a, vec2<float> b, int brushSize,
                                  int maxW, int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize);

  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}
void Line::onMouseDown(vec2<float> pos, Canvas &canvas) {
  drawing = true;
  Start = pos;
  Last = pos;

  currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);
  prevBound = computeLineBounds(pos, pos, brushSize, canvas.w, canvas.h);

  Logger::log(LogLevel::DEBUG, "LINE TOOL: START");
}

void Line::onMouseMove(vec2<float> pos, Canvas &canvas) {
  if (!drawing)
    return;

  SDL_BlitSurface(currentSnapshot, &prevBound, canvas.drawingSurface,
                  &prevBound);

  SDL_Rect newBound =
      computeLineBounds(Start, pos, brushSize, canvas.w, canvas.h);

  prevBound = newBound;
  Last = pos;

  auto s1 = std::chrono::high_resolution_clock::now();
  Renderer::bresenham(Start, pos, canvas, color, brushSize, false);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  // Renderer::dda(Start, pos, canvas, color, brushSize, false);
  auto e2 = std::chrono::high_resolution_clock::now();

  float tBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float tDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

  Profiler::recordRaceStep({{"BRESENHAM", tBres, ImVec4(1, 0.5f, 0, 1)},
                            {"DDA", tDDA, ImVec4(0, 0.5f, 1, 1)}},
                           pos);
}

Command *Line::onMouseUp(vec2<float> pos, Canvas &canvas) {
  drawing = false;

  SDL_BlitSurface(currentSnapshot, &prevBound, canvas.drawingSurface,
                  &prevBound);

  Renderer::bresenham(Start, pos, canvas, color, brushSize, false);

  SDL_Rect finalBound =
      computeLineBounds(Start, pos, brushSize, canvas.w, canvas.h);

  DrawCommand *cmd = new DrawCommand(canvas.drawingSurface, finalBound);
  cmd->captureAfter(canvas.drawingSurface);

  freeSnapshot();

  Profiler::commitRace(
      {{"BRESENHAM", ImVec4(1, 0.5f, 0, 1)}, {"DDA", ImVec4(0, 0.5f, 1, 1)}});

  return cmd;
}
