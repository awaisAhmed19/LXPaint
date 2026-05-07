#include "Rect.h"
#include <algorithm>
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize);

  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}
void RectFill(Canvas &canvas, int minX, int minY, int maxX, int maxY,
              uint32_t color) {
  for (int x = minX; x <= maxX; x++) {
    for (int y = minY; y <= maxY; y++) {
      canvas.drawPixel(x, y, color);
    }
  }
}

void RectFillWhite(Canvas &canvas, int minX, int minY, int maxX, int maxY) {
  const uint32_t color = 0xFFFFFFFF;
  for (int x = minX + 1; x <= maxX - 1; x++) {
    for (int y = minY + 1; y <= maxY - 1; y++) {
      canvas.drawPixel(x, y, color);
    }
  }
}
static void drawRect(vec2 a, vec2 b, Canvas &canvas, uint32_t color,
                     int brushSize) {

  int minX = std::min((int)a.x, (int)b.x);
  int minY = std::min((int)a.y, (int)b.y);
  int maxX = std::max((int)a.x, (int)b.x);
  int maxY = std::max((int)a.y, (int)b.y);

  // Top
  Renderer::bresenham({(float)minX, (float)minY}, {(float)maxX, (float)minY},
                      canvas.m_canvasSurface, color, brushSize, false);
  // Bottom
  Renderer::bresenham({(float)minX, (float)maxY}, {(float)maxX, (float)maxY},
                      canvas.m_canvasSurface, color, brushSize, false);

  // Left
  Renderer::bresenham({(float)minX, (float)minY}, {(float)minX, (float)maxY},
                      canvas.m_canvasSurface, color, brushSize, false);

  // Right
  Renderer::bresenham({(float)maxX, (float)minY}, {(float)maxX, (float)maxY},
                      canvas.m_canvasSurface, color, brushSize, false);
  // RectFill(canvas, minX, minY, maxX, maxY, color);
  RectFillWhite(canvas, minX, minY, maxX, maxY);
}

void Rect::onMouseDown(vec2 pos, Canvas &canvas) {
  drawing = true;
  Start = pos;
  Last = pos;

  currentSnapshot = SDL_DuplicateSurface(canvas.m_canvasSurface);
  prevBound = computeRectBounds(pos, pos, brushSize, canvas.getWidth(),
                                canvas.getHeight());

  Logger::log(LogLevel::DEBUG, "RECT TOOL: START");
}

void Rect::onMouseMove(vec2 pos, Canvas &canvas) {
  if (!drawing)
    return;

  SDL_BlitSurface(currentSnapshot, &prevBound, canvas.m_canvasSurface,
                  &prevBound);

  SDL_Rect newBound = computeRectBounds(Start, pos, brushSize,
                                        canvas.getWidth(), canvas.getHeight());

  prevBound = newBound;
  Last = pos;

  auto s1 = std::chrono::high_resolution_clock::now();
  drawRect(Start, pos, canvas, color, brushSize);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  auto e2 = std::chrono::high_resolution_clock::now();

  float tBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float tDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

  Profiler::recordRaceStep({{"BRESENHAM", tBres, ImVec4(1, 0.5f, 0, 1)},
                            {"DDA", tDDA, ImVec4(0, 0.5f, 1, 1)}},
                           pos);
}

Command *Rect::onMouseUp(vec2 pos, Canvas &canvas) {
  drawing = false;

  SDL_BlitSurface(currentSnapshot, &prevBound, canvas.m_canvasSurface,
                  &prevBound);

  drawRect(Start, pos, canvas, color, brushSize);

  SDL_Rect finalBound = computeRectBounds(
      Start, pos, brushSize, canvas.getWidth(), canvas.getHeight());

  DrawCommand *cmd = new DrawCommand(canvas.m_canvasSurface, finalBound);
  cmd->captureAfter(canvas.m_canvasSurface);

  freeSnapshot();

  Profiler::commitRace(
      {{"BRESENHAM", ImVec4(1, 0.5f, 0, 1)}, {"DDA", ImVec4(0, 0.5f, 1, 1)}});

  return cmd;
}
