#include <SDL3/SDL_rect.h>
#include <algorithm>

#include "Document/PreviewLayer.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Editor/ToolSettings.h"
#include "Rect.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"

#include "Document/PreviewLayer.h"
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize - 4);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize - 4);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize + 4);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize + 4);
  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

void Rect::onMouseDown(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "RECT TOOL: START");

  ctx.interaction->active = true;

  m_start = pos;
  m_last = pos;
  m_affected =
      computeRectBounds(pos, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);
}

void Rect::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  m_last = pos;

  m_affected =
      computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  ctx.preview->clearRGBA(0, 0, 0, 0);
  switch (ctx.settings->fillmode) {
  case ToolSettings::FillMode::Outline:
    Rasterizer::drawRectStroke(ctx.preview->getSurface(), m_start, pos,
                               ctx.fgColor, brushSize);
    break;
  case ToolSettings::FillMode::Opaque:
    Rasterizer::drawRectStroke(ctx.preview->getSurface(), m_start, pos,
                               ctx.fgColor, brushSize);
    Rasterizer::rectFillWhite(ctx.preview->getSurface(), (int)m_start.x + 1,
                              (int)m_start.y + 1, (int)pos.x - 1,
                              (int)pos.y - 1);
    break;
  case ToolSettings::FillMode::Fill:
    Rasterizer::rectFill(ctx.preview->getSurface(), (int)m_start.x,
                         (int)m_start.y, (int)pos.x, (int)pos.y, ctx.fgColor);
    break;
  }

  ctx.preview->invalidateRect(m_affected);
}

std::unique_ptr<Command> Rect::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "RECT TOOL: END");

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
  case ToolSettings::FillMode::Outline:
    Rasterizer::drawRectStroke(ctx.canvas->getSurface(), m_start, pos,
                               ctx.fgColor, brushSize);
    break;
  case ToolSettings::FillMode::Opaque:
    Rasterizer::drawRectStroke(ctx.canvas->getSurface(), m_start, pos,
                               ctx.fgColor, brushSize);
    Rasterizer::rectFillWhite(ctx.canvas->getSurface(), (int)m_start.x + 1,
                              (int)m_start.y + 1, (int)pos.x - 1,
                              (int)pos.y - 1);
    break;
  case ToolSettings::FillMode::Fill:
    Rasterizer::rectFill(ctx.canvas->getSurface(), (int)m_start.x,
                         (int)m_start.y, (int)pos.x, (int)pos.y, ctx.fgColor);
    break;
  }

  ctx.canvas->invalidateRect(m_affected);

  m_command->captureAfter(ctx.canvas->getSurface());

  return std::move(m_command);
}
