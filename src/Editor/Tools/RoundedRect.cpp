#include <SDL3/SDL_rect.h>
#include <algorithm>

#include "Document/PreviewLayer.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "RoundedRect.h"
#include "Systems/Logger.h"

#include "Document/PreviewLayer.h"
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize - 4);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize - 4);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize + 4);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize + 4);

  return SDL_Rect{
      minX,
      minY,
      maxX - minX + 1,
      maxY - minY + 1,
  };
}

void RoundedRect::onMouseDown(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "ROUNDED RECT TOOL: START");

  ctx.interaction->active = true;

  m_start = pos;
  m_last = pos;
  m_affected =
      computeRectBounds(pos, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);
}

void RoundedRect::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  m_last = pos;

  m_affected =
      computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  ctx.preview->clearRGBA(0, 0, 0, 0);

  switch (ctx.settings->fillmode) {
  case ToolSettings::FillMode::Fill: {
    Rasterizer::drawRoundedRect(ctx.preview->getSurface(), m_start, pos,
                                ctx.fgColor, brushSize, m_points);
    Rasterizer::fillPolygon(ctx.preview->getSurface(), m_points, ctx.fgColor);
  } break;

  case ToolSettings::FillMode::Opaque: {
    Rasterizer::drawRoundedRect(ctx.preview->getSurface(), m_start, pos,
                                ctx.fgColor, 4, m_points);
    Rasterizer::fillPolygon(ctx.preview->getSurface(), m_points, 0xFFFFFFFF);
  } break;
  case ToolSettings::FillMode::Outline:
    Rasterizer::drawRoundedRect(ctx.preview->getSurface(), m_start, pos,
                                ctx.fgColor, brushSize, m_points);
    break;
  }
  ctx.preview->invalidateRect(m_affected);
}

std::unique_ptr<Command> RoundedRect::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "ROUNDED RECT TOOL: END");

  if (!ctx.interaction->active)
    return nullptr;

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(m_affected);

  m_last = pos;

  m_affected =
      computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  m_command =
      std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), m_affected);

  switch (ctx.settings->fillmode) {
  case ToolSettings::FillMode::Outline: {
    Rasterizer::drawRoundedRect(ctx.canvas->getSurface(), m_start, pos,
                                ctx.fgColor, brushSize, m_points);
  } break;
  case ToolSettings::FillMode::Fill: {
    Rasterizer::drawRoundedRect(ctx.canvas->getSurface(), m_start, pos,
                                ctx.fgColor, 2, m_points);
    Rasterizer::fillPolygon(ctx.canvas->getSurface(), m_points, ctx.fgColor);
  } break;
  case ToolSettings::FillMode::Opaque: {
    Rasterizer::drawRoundedRect(ctx.canvas->getSurface(), m_start, pos,
                                ctx.fgColor, 4, m_points);
    Rasterizer::fillPolygon(ctx.canvas->getSurface(), m_points, 0xFFFFFFFF);
  } break;
  }

  ctx.canvas->invalidateRect(m_affected);

  m_command->captureAfter(ctx.canvas->getSurface());

  return std::move(m_command);
}
