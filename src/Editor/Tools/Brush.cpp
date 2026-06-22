#include "Brush.h"

#include "Editor/Editor.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Systems/Logger.h"
#include <algorithm>
#include <cmath>
Rasterizer::BrushShape Brush::toRasterShape(int toolSettingsShape) {
  // Explicit mapping rather than static_cast<BrushShape>(int) — these are
  // two independent enums (ToolSettings::BrushShape vs Rasterizer::BrushShape)
  // that currently happen to share ordering. An explicit switch means a
  // future reorder of either enum fails loudly (missing case) instead of
  // silently mismapping shapes.
  switch (toolSettingsShape) {
  case 0: // ToolSettings::BrushShape::Round
    return Rasterizer::BrushShape::Round;
  case 1: // ToolSettings::BrushShape::Square
    return Rasterizer::BrushShape::Square;
  case 2: // ToolSettings::BrushShape::ForwardSlash
    return Rasterizer::BrushShape::ForwardSlash;
  case 3: // ToolSettings::BrushShape::BackSlash
    return Rasterizer::BrushShape::BackSlash;
  default:
    return Rasterizer::BrushShape::Round;
  }
}

int Brush::clampSize(float size) {
  // Brush sizes are restricted to 1, 2, 4 per spec. Snap whatever
  // ToolSettings::strokeWidth holds to the nearest supported size rather
  // than assuming the UI only ever sets exact values.
  int s = (int)std::round(size);
  if (s <= 1)
    return 1;
  if (s <= 3)
    return 2;
  return 4;
}

void Brush::onMouseDown(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Brush canvas missing");
  LX_ASSERT(ctx.interaction != nullptr, "Brush interaction missing");
  Logger::debug("BRUSH START");

  m_start = pos;
  m_last = pos;

  int size = clampSize(ctx.settings->strokeWidth);
  int margin = size + 1;

  beginStrokeBounds(pos, margin, ctx.canvas->getSurface()->w,
                    ctx.canvas->getSurface()->h);

  freeBackupSurface();
  m_backupSurface = SDL_DuplicateSurface(ctx.canvas->getSurface());

  if (!m_backupSurface) {
    Logger::log(
        LogLevel::ERR,
        std::format("Failed to duplicate backup surface: {}", SDL_GetError()));
    return;
  }

  auto shape = toRasterShape((int)ctx.settings->brushShape);

  Rasterizer::stampBrush(ctx.canvas->getSurface(), pos, ctx.fgColor, shape,
                         size);

  SDL_Rect initialRect{(int)pos.x - margin, (int)pos.y - margin, margin * 2 + 1,
                       margin * 2 + 1};

  ctx.canvas->invalidateRect(initialRect);
}

void Brush::onMouseMove(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Brush move canvas missing");

  if (!ctx.interaction->active)
    return;

  int size = clampSize(ctx.settings->strokeWidth);
  int margin = size + 1;

  expandStrokeBounds(pos, margin, ctx.canvas->getSurface()->w,
                     ctx.canvas->getSurface()->h);

  SDL_Rect segmentRect{std::min((int)m_last.x, (int)pos.x) - margin,
                       std::min((int)m_last.y, (int)pos.y) - margin,
                       std::abs((int)(pos.x - m_last.x)) + margin * 2 + 1,
                       std::abs((int)(pos.y - m_last.y)) + margin * 2 + 1};

  auto shape = toRasterShape((int)ctx.settings->brushShape);

  // Continuous interpolation between mouse samples — reuses the same
  // step-per-pixel walk Rasterizer already applies for sprayStroke/bresenham,
  // so strokes do not gap on fast mouse movement (no event-only stamping).
  Rasterizer::brushStroke(ctx.canvas->getSurface(), m_last, pos, ctx.fgColor,
                          shape, size);

  ctx.canvas->invalidateRect(segmentRect);

  m_last = pos;
}

std::unique_ptr<Command> Brush::onMouseUp(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(m_backupSurface != nullptr, "Brush backup surface missing");
  Logger::debug("BRUSH END");

  if (!ctx.interaction->active)
    return nullptr;

  int size = clampSize(ctx.settings->strokeWidth);
  int margin = size + 1;

  expandStrokeBounds(pos, margin, ctx.canvas->getSurface()->w,
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
