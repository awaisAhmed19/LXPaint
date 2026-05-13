#include "Pencil.h"
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"
#include "../Preview/PreviewLayer.h"
void Pencil::onMouseDown(vec2 pos, ToolContext &ctx) {
  m_last = pos;
  resetBounds(pos, brushSize);
  m_pendingCommand =
      std::make_unique<DrawCommand>(ctx.canvas->getSurface(), m_boundingBox);
  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);
  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->markDirty();
  ctx.canvas->markDirty();
  Logger::log(LogLevel::DEBUG, "PENCIL STARTED DRAWING");
}

void Pencil::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  // Abstracted Logic
  updateBounds(pos, brushSize, ctx.canvas->getSurface()->w,
               ctx.canvas->getSurface()->h);
  // ps->clear();
  // Benchmarking
  auto s1 = std::chrono::high_resolution_clock::now();
  Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  // Rasterizer::dda(Start, pos, canvas, color, brushSize, useXOR);
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

  SDL_Log("DIRTY AFTER MOVE: %d", ctx.canvas->isDirty());
  m_last = pos;
  // ps->sync();
}

std::unique_ptr<Command> Pencil::onMouseUp(vec2 pos, ToolContext &ctx) {
  ctx.interaction->active = false;
  if (m_pendingCommand) {
    m_pendingCommand->captureAfter(ctx.canvas->getSurface());
  }

  // Finalize Profiler
  Profiler::commitRace({{"BRESENHAM", ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
                        {"DDA", ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}});

  Logger::log(LogLevel::DEBUG, "PENCIL STOPPED DRAWING");
  return std::move(m_pendingCommand);
}
