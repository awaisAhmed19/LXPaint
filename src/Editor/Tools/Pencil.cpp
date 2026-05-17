#include "Pencil.h"

#include "../Commands/SnapshotCommand.h"
#include "../Interaction/ToolContext.h"
#include "../Interaction/ToolInteractionState.h"
void Pencil::onMouseDown(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Pencil canvas missing");
  LX_ASSERT(ctx.interaction != nullptr, "Pencil interaction missing");
  Logger::debug("PENCIL START");
  m_start = pos;
  m_last = pos;

  resetBounds(pos, brushSize);

  if (m_backupSurface) {
    SDL_DestroySurface(m_backupSurface);
    m_backupSurface = nullptr;
  }

  m_backupSurface = SDL_DuplicateSurface(ctx.canvas->getSurface());

  if (!m_backupSurface) {

    Logger::log(
        LogLevel::ERR,
        std::format("Failed to duplicate backup surface: {}", SDL_GetError()));

    return;
  }

  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), color, brushSize,
                        false);

  ctx.canvas->markDirty();
}

void Pencil::onMouseMove(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Pencil move canvas missing");
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
  LX_ASSERT(m_backupSurface != nullptr, "Pencil backup surface missing");
  Logger::debug("PENCIL END");

  if (!ctx.interaction->active)
    return nullptr;

  updateBounds(pos, brushSize, ctx.canvas->getSurface()->w,
               ctx.canvas->getSurface()->h);

  LX_ASSERT(m_boundingBox.w > 0 && m_boundingBox.h > 0, "Invalid bounding box");
  auto before = SnapshotCommand::copyRegion(m_backupSurface, m_boundingBox);

  auto after =
      SnapshotCommand::copyRegion(ctx.canvas->getSurface(), m_boundingBox);

  if (!before || !after) {

    Logger::log(LogLevel::ERR, "Snapshot region capture failed");

    SDL_DestroySurface(m_backupSurface);
    m_backupSurface = nullptr;

    return nullptr;
  }

  auto cmd = std::make_unique<SnapshotCommand>(std::move(before),
                                               std::move(after), m_boundingBox);

  SDL_DestroySurface(m_backupSurface);
  m_backupSurface = nullptr;

  return cmd;
}
