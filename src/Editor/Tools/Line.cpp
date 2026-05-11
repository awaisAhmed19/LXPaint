#include "Line.h"
#include "../Commands/DrawCommand.h"
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"
#include "../Preview/PreviewLayer.h"
#include <algorithm>
static SDL_Rect computeLineBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize);

  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}
void Line::onMouseDown(vec2 pos, ToolContext &ctx) {
  ctx.interaction->active = true;
  m_start = pos;
  m_last = pos;

  Logger::log(LogLevel::DEBUG, "LINE TOOL: START");
}

void Line::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  ctx.preview->clear(0x00000000);

  m_boundingBox =
      computeLineBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  auto s1 = std::chrono::high_resolution_clock::now();

  Rasterizer::bresenham(m_start, pos, ctx.preview->getSurface(), color,
                        brushSize, false);

  auto e1 = std::chrono::high_resolution_clock::now();

  float tBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();

  ctx.preview->markDirty();

  Profiler::recordRaceStep({{"BRESENHAM", tBres, ImVec4(1, 0.5f, 0, 1)}}, pos);
}

std::unique_ptr<Command> Line::onMouseUp(vec2 pos, ToolContext &ctx) {
  ctx.interaction->active = false;
  ctx.preview->clear(0x00000000);
  ctx.preview->markDirty();
  m_boundingBox =
      computeLineBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  m_last = pos;
  auto cmd =
      std::make_unique<DrawCommand>(ctx.canvas->getSurface(), m_boundingBox);
  Rasterizer::bresenham(m_start, pos, ctx.canvas->getSurface(), color,
                        brushSize, false);
  ctx.canvas->markDirty();
  cmd->captureAfter(ctx.canvas->getSurface());

  Profiler::commitRace(
      {{"BRESENHAM", ImVec4(1, 0.5f, 0, 1)}, {"DDA", ImVec4(0, 0.5f, 1, 1)}});

  return cmd;
}
