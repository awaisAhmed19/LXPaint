#include "AirBrush.h"

#include "App/Globals.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Editor.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"

#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
#include <cmath>
static float distance(vec2 start, vec2 end) {
  float x_prime = end.x - start.x;
  float y_prime = end.y - start.y;

  return std::sqrtf(std::powf(x_prime, 2.f) + std::powf(y_prime, 2.f));
}
void AirBrush::onMouseDown(vec2 pos, ToolContext &ctx) {

  Logger::debug("AirBrush START");

  auto &settings = *ctx.settings;

  m_start = pos;
  m_last = pos;

  int margin = static_cast<int>(std::ceil(settings.airbrushRadius));

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

  Rasterizer::spray(ctx.canvas->getSurface(), pos, ctx.fgColor,
                    settings.airbrushRadius, settings.airbrushDensity);

  SDL_Rect dirty{int(pos.x) - margin, int(pos.y) - margin, margin * 2 + 1,
                 margin * 2 + 1};

  ctx.canvas->invalidateRect(dirty);
}

void AirBrush::onMouseMove(vec2 pos, ToolContext &ctx) {

  if (!ctx.interaction->active)
    return;

  auto &settings = *ctx.settings;

  int margin = static_cast<int>(std::ceil(settings.airbrushRadius));

  expandStrokeBounds(pos, margin, ctx.canvas->getSurface()->w,
                     ctx.canvas->getSurface()->h);

  Rasterizer::sprayStroke(ctx.canvas->getSurface(), m_last, pos, ctx.fgColor,
                          settings.airbrushRadius, settings.airbrushDensity);

  SDL_Rect dirty{std::min(int(m_last.x), int(pos.x)) - margin,
                 std::min(int(m_last.y), int(pos.y)) - margin,
                 std::abs(int(pos.x - m_last.x)) + margin * 2 + 1,
                 std::abs(int(pos.y - m_last.y)) + margin * 2 + 1};

  ctx.canvas->invalidateRect(dirty);

  m_last = pos;
}
std::unique_ptr<Command> AirBrush::onMouseUp(vec2 pos, ToolContext &ctx) {

  Logger::debug("AirBrush END");

  if (!ctx.interaction->active)
    return nullptr;

  auto &settings = *ctx.settings;

  int margin = static_cast<int>(std::ceil(settings.airbrushRadius));

  expandStrokeBounds(pos, margin, ctx.canvas->getSurface()->w,
                     ctx.canvas->getSurface()->h);

  auto before = SnapshotCommand::copyRegion(m_backupSurface, m_strokeBounds);

  auto after =
      SnapshotCommand::copyRegion(ctx.canvas->getSurface(), m_strokeBounds);

  freeBackupSurface();

  m_hasStrokeBounds = false;

  if (!before || !after) {
    Logger::log(LogLevel::ERR, "Snapshot region capture failed");
    return nullptr;
  }

  return std::make_unique<SnapshotCommand>(std::move(before), std::move(after),
                                           m_strokeBounds);
}
