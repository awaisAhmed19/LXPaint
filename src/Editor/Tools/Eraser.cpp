#include "Eraser.h"

#include "../Commands/DrawCommand.h"
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"
void Eraser::onMouseDown(vec2 pos, ToolContext &ctx) {
  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), m_color, brushSize,
                        m_useXOR);
  ctx.canvas->markDirty();
  Logger::log(LogLevel::DEBUG, "ERASER STARTED DRAWING");
}

void Eraser::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  // Abstracted Logic
  updateBounds(pos, brushSize, ctx.canvas->getSurface()->w,
               ctx.canvas->getSurface()->h);
  // ps->clear();
  // Benchmarking
  auto s1 = std::chrono::high_resolution_clock::now();

  Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), m_color,
                        brushSize, false);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  // Rasterizer::dda(Start, pos, canvas, m_color, brushSize, useXOR);
  auto e2 = std::chrono::high_resolution_clock::now();

  float usBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float usDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

  Profiler::recordRaceStep(
      {{"BRESENHAM", usBres, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
       {"DDA", usDDA, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}},
      pos);
  ctx.canvas->markDirty();
  m_last = pos;
  // ps->sync();
}

std::unique_ptr<Command> Eraser::onMouseUp(vec2 pos, ToolContext &ctx) {
  ctx.interaction->active = false;
  auto cmd =
      std::make_unique<DrawCommand>(ctx.canvas->getSurface(), m_boundingBox);
  ctx.canvas->markDirty();
  cmd->captureAfter(ctx.canvas->getSurface());

  // Finalize Profiler
  Profiler::commitRace({{"BRESENHAM", ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
                        {"DDA", ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}});

  Logger::log(LogLevel::DEBUG, "ERASER STOPPED DRAWING");
  return std::move(cmd);
}
