#include "Line.h"

#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Editor/ToolSettings.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
#include <algorithm>

#include "Document/PreviewLayer.h"
static SDL_Rect computeLineBounds(vec2 a, vec2 b, int brushSize, int maxW,
                                  int maxH) {
  int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize);
  int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize);
  int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize);
  int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize);
  return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

void Line::onMouseDown(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "LINE TOOL: START");

  ctx.interaction->active = true;

  m_start = pos;
  m_last = pos;

  m_affected =
      computeLineBounds(pos, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);
}

void Line::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;
  m_last = pos;
  m_affected =
      computeLineBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);
  ctx.preview->clearRGBA(0, 0, 0, 0);
  Rasterizer::bresenham(m_start, pos, ctx.preview->getSurface(), ctx.fgColor,
                        ctx.settings->lineWidth, false);
  ctx.preview->invalidateRect(m_affected);
}

std::unique_ptr<Command> Line::onMouseUp(vec2 pos, ToolContext &ctx) {
  Logger::log(LogLevel::DEBUG, "LINE TOOL: END");
  if (!ctx.interaction->active)
    return nullptr;
  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(m_affected);
  m_last = pos;

  m_affected =
      computeLineBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                        ctx.canvas->getSurface()->h);
  m_command =
      std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), m_affected);

  Rasterizer::bresenham(m_start, pos, ctx.canvas->getSurface(), ctx.fgColor,
                        ctx.settings->lineWidth, false);
  ctx.canvas->invalidateRect(m_affected);
  m_command->captureAfter(ctx.canvas->getSurface());
  return std::move(m_command);
}
