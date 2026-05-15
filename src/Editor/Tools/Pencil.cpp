#include "Pencil.h"
#include "../Commands/DrawStrokeCommand.h" // ✅ Add include
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"
#include "../Preview/PreviewLayer.h"

// Pencil::onMouseDown()
void Pencil::onMouseDown(vec2 pos, ToolContext &ctx) {
  m_last = pos;
  m_activeCommand = std::make_unique<DrawStrokeCommand>();

  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);
  ctx.canvas->markDirty(); // ✅ Mark dirty
  Logger::log(LogLevel::DEBUG, "PENCIL STARTED DRAWING");
}

// Pencil::onMouseMove()
void Pencil::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active || !m_activeCommand)
    return;

  m_activeCommand->addSegment(m_last, pos, color, brushSize);
  Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);
  ctx.canvas->markDirty(); // ✅ Mark dirty each move

  // ... profiler stuff ...
  m_last = pos;
}

// Pencil.cpp
std::unique_ptr<Command> Pencil::onMouseUp(vec2 pos, ToolContext &ctx) {
  ctx.interaction->active = false;

  if (!m_activeCommand || m_activeCommand->isEmpty()) {
    return nullptr;
  }

  size_t estimateMemory = m_activeCommand->getSegmentCount() * 32;

  Profiler::commitRace({{"BRESENHAM", ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
                        {"DDA", ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}});

  Logger::log(LogLevel::DEBUG, "PENCIL STOPPED DRAWING");
  return std::move(m_activeCommand);
}
