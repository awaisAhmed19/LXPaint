#include "Pencil.h"

#include "../Commands/SnapshotCommand.h"
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"

void Pencil::onMouseDown(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "PENCIL STARTED DRAWING");

  ctx.interaction->active = true;

  m_start = pos;
  m_last = pos;

  resetBounds(pos, brushSize);

  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);

  ctx.canvas->markDirty();
}

void Pencil::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  updateBounds(pos, brushSize, ctx.canvas->getSurface()->w,
               ctx.canvas->getSurface()->h);

  Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);

  ctx.canvas->markDirty();

  m_last = pos;
}

std::unique_ptr<Command> Pencil::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "PENCIL STOPPED DRAWING");

  if (!ctx.interaction->active)
    return nullptr;

  ctx.interaction->active = false;

  if (m_last != pos) {
    updateBounds(pos, brushSize, ctx.canvas->getSurface()->w,
                 ctx.canvas->getSurface()->h);

    m_command = std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(),
                                                  m_boundingBox);
    Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), color,
                          brushSize, false);

    ctx.canvas->markDirty();
  }

  m_command->captureAfter(ctx.canvas->getSurface());

  return std::move(m_command);
}
