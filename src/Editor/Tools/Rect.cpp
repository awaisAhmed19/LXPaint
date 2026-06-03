#include <SDL3/SDL_rect.h>
#include <algorithm>

#include "Document/PreviewLayer.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rect.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
static SDL_Rect affected{0};
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
  affected = computeRectBounds(pos, pos, brushSize, ctx.canvas->getSurface()->w,
                               ctx.canvas->getSurface()->h);
}

void Rect::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  m_last = pos;

  affected =
      computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  ctx.preview->clearRGBA(0, 0, 0, 0);

  Rasterizer::drawRectStroke(ctx.preview->getSurface(), m_start, pos, color,
                             brushSize);

  ctx.preview->invalidateRect(affected);
}

std::unique_ptr<Command> Rect::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "RECT TOOL: END");

  if (!ctx.interaction->active)
    return nullptr;

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(affected);

  m_last = pos;

  affected =
      computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);

  m_command =
      std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), affected);

  Rasterizer::drawRectStroke(ctx.canvas->getSurface(), m_start, pos, color,
                             brushSize);

  ctx.canvas->invalidateRect(affected);

  m_command->captureAfter(ctx.canvas->getSurface());

  return std::move(m_command);
}
