#include "Pencil.h"

#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"

#include "Rendering/Rasterizer.h"

#include "Systems/Logger.h"

void Pencil::onMouseDown(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Pencil canvas missing");

  LX_ASSERT(ctx.interaction != nullptr, "Pencil interaction missing");

  Logger::debug("PENCIL START");

  m_start = pos;
  m_last = pos;

  beginStrokeBounds(pos, ctx.brushSize, ctx.canvas->getSurface()->w,
                    ctx.canvas->getSurface()->h);

  freeBackupSurface();

  m_backupSurface = SDL_DuplicateSurface(ctx.canvas->getSurface());

  if (!m_backupSurface) {

    Logger::log(
        LogLevel::ERR,
        std::format("Failed to duplicate backup surface: {}", SDL_GetError()));

    return;
  }

  Rasterizer::bresenham(pos, pos, ctx.canvas->getSurface(), ctx.fgColor,
                        ctx.brushSize, false);

  SDL_Rect initialRect{(int)pos.x - ctx.brushSize, (int)pos.y - ctx.brushSize,
                       ctx.brushSize * 2 + 1, ctx.brushSize * 2 + 1};
  SDL_Log("fgColor = %08X", ctx.fgColor);
  ctx.canvas->invalidateRect(initialRect);
}

void Pencil::onMouseMove(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Pencil move canvas missing");

  if (!ctx.interaction->active)
    return;

  expandStrokeBounds(pos, ctx.brushSize, ctx.canvas->getSurface()->w,
                     ctx.canvas->getSurface()->h);

  SDL_Rect segmentRect{
      std::min((int)m_last.x, (int)pos.x) - ctx.brushSize,
      std::min((int)m_last.y, (int)pos.y) - ctx.brushSize,
      std::abs((int)(pos.x - m_last.x)) + ctx.brushSize * 2 + 1,
      std::abs((int)(pos.y - m_last.y)) + ctx.brushSize * 2 + 1};

  Rasterizer::bresenham(m_last, pos, ctx.canvas->getSurface(), ctx.fgColor,
                        ctx.brushSize, false);

  ctx.canvas->invalidateRect(segmentRect);

  m_last = pos;
}

std::unique_ptr<Command> Pencil::onMouseUp(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(m_backupSurface != nullptr, "Pencil backup surface missing");

  Logger::debug("PENCIL END");

  if (!ctx.interaction->active)
    return nullptr;

  expandStrokeBounds(pos, ctx.brushSize, ctx.canvas->getSurface()->w,
                     ctx.canvas->getSurface()->h);

  LX_ASSERT(m_hasStrokeBounds, "Invalid stroke bounds");

  auto before = SnapshotCommand::copyRegion(m_backupSurface, m_strokeBounds);

  auto after =
      SnapshotCommand::copyRegion(ctx.canvas->getSurface(), m_strokeBounds);

  if (!before || !after) {

    Logger::log(LogLevel::ERR, "Snapshot region capture failed");

    freeBackupSurface();

    return nullptr;
  }

  auto cmd = std::make_unique<SnapshotCommand>(
      std::move(before), std::move(after), m_strokeBounds);

  freeBackupSurface();

  m_hasStrokeBounds = false;

  return cmd;
}
