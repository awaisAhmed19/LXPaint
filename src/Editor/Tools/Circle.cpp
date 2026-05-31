
#include <algorithm>
#include <cmath>

#include "Circle.h"
#include "Document/PreviewLayer.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
static SDL_Rect computeEllipseBounds(vec2 center, int rx, int ry, int padding,
                                     int maxW, int maxH) {
  int minX = std::max(0, (int)(center.x - rx - padding));
  int minY = std::max(0, (int)(center.y - ry - padding));

  int maxX = std::min(maxW - 1, (int)(center.x + rx + padding));
  int maxY = std::min(maxH - 1, (int)(center.y + ry + padding));

  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

void Circle::onMouseDown(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "CIRCLE TOOL: START");

  ctx.interaction->active = true;

  m_start = pos;
  m_last = pos;
}

void Circle::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  m_last = pos;

  ctx.preview->clearRGBA(0, 0, 0, 0);
  int dx = (int)(pos.x - m_start.x);
  int dy = (int)(pos.y - m_start.y);

  int rx = (int)(pos.x - m_start.x);
  int ry = (int)(pos.y - m_start.y);

  ctx.canvas->m_boundingBox = computeEllipseBounds(
      m_start, std::abs(rx), std::abs(ry), brushSize,
      ctx.canvas->getSurface()->w, ctx.canvas->getSurface()->h);

  Rasterizer::drawEllipse_theta(ctx.preview->getSurface(), (int)m_start.x,
                                (int)m_start.y, rx, ry, color);

  ctx.preview->markDirty();
}

std::unique_ptr<Command> Circle::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "CIRCLE TOOL: END");

  if (!ctx.interaction->active)
    return nullptr;

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->markDirty();
  m_last = pos;

  int dx = (int)(pos.x - m_start.x);
  int dy = (int)(pos.y - m_start.y);

  int rx = (int)(pos.x - m_start.x);
  int ry = (int)(pos.y - m_start.y);

  SDL_Rect affected = computeEllipseBounds(
      m_start, std::abs(rx), std::abs(ry), brushSize,
      ctx.canvas->getSurface()->w, ctx.canvas->getSurface()->h);

  m_command =
      std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), affected);
  Rasterizer::drawEllipse_theta(ctx.canvas->getSurface(), (int)m_start.x,
                                (int)m_start.y, rx, ry, color);
  // TODO:add settings to tools so that we can distiguish the stoke, fill etc
  ctx.canvas->invalidateRect(affected);
  m_command->captureAfter(ctx.canvas->getSurface());

  return std::move(m_command);
}
