
#include <algorithm>
#include <cmath>

#include "Circle.h"
#include "Document/PreviewLayer.h"
#include "Editor/Commands/SnapshotCommand.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
static SDL_Rect computeRectBounds(vec2 a, vec2 b, int brushSize, int maxW, int maxH) {
    int minX = std::max(0, std::min((int)a.x, (int)b.x) - brushSize - 4);
    int minY = std::max(0, std::min((int)a.y, (int)b.y) - brushSize - 4);
    int maxX = std::min(maxW - 1, std::max((int)a.x, (int)b.x) + brushSize + 4);
    int maxY = std::min(maxH - 1, std::max((int)a.y, (int)b.y) + brushSize + 4);
    return SDL_Rect{minX, minY, maxX - minX + 1, maxY - minY + 1};
}

void Circle::onMouseDown(vec2 pos, ToolContext& ctx) {
    Logger::log(LogLevel::DEBUG, "RECT TOOL: START");

    ctx.interaction->active = true;

    m_start = pos;
    m_last = pos;

    m_boundingBox = computeRectBounds(pos, pos, brushSize, ctx.canvas->getSurface()->w,
                                      ctx.canvas->getSurface()->h);
}

void Circle::onMouseMove(vec2 pos, ToolContext& ctx) {
    if (!ctx.interaction->active) return;

    m_last = pos;

    m_boundingBox = computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                                      ctx.canvas->getSurface()->h);

    ctx.preview->clearRGBA(0, 0, 0, 0);
    int dx = (int)(pos.x - m_start.x);
    int dy = (int)(pos.y - m_start.y);

    int rx = std::abs(pos.x - m_start.x);
    int ry = std::abs(pos.y - m_start.y);

    Rasterizer::drawEllipse(ctx.preview->getSurface(), (int)m_start.x, (int)m_start.y, rx, ry,
                            color);

    ctx.preview->markDirty();
}

std::unique_ptr<Command> Circle::onMouseUp(vec2 pos, ToolContext& ctx) {
    Logger::log(LogLevel::DEBUG, "CIRCLE TOOL: END");

    if (!ctx.interaction->active) return nullptr;

    ctx.interaction->active = false;

    ctx.preview->clearRGBA(0, 0, 0, 0);
    ctx.preview->markDirty();
    m_last = pos;

    m_boundingBox = computeRectBounds(m_start, pos, brushSize, ctx.canvas->getSurface()->w,
                                      ctx.canvas->getSurface()->h);

    m_command = std::make_unique<SnapshotCommand>(ctx.canvas->getSurface(), m_boundingBox);
    int dx = (int)(pos.x - m_start.x);
    int dy = (int)(pos.y - m_start.y);

    int rx = std::abs(pos.x - m_start.x);
    int ry = std::abs(pos.y - m_start.y);

    Rasterizer::drawEllipse(ctx.canvas->getSurface(), (int)m_start.x, (int)m_start.y, rx, ry,
                            color);

    ctx.canvas->markDirty();
    m_command->captureAfter(ctx.canvas->getSurface());

    return std::move(m_command);
}
