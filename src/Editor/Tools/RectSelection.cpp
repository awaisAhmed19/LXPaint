#include "RectSelection.h"
#include "Editor/Commands/Command.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"

#include <algorithm>

bool RectSelection::containsPoint(vec2 pos) const {
  if (m_selection.empty())
    return false;

  int lx = (int)pos.x - m_selection.bounds.x - (int)m_selection.offset.x;
  int ly = (int)pos.y - m_selection.bounds.y - (int)m_selection.offset.y;

  return lx >= 0 && ly >= 0 && lx < m_selection.bounds.w &&
         ly < m_selection.bounds.h;
}

void RectSelection::buildMask() {
  m_selection.mask.assign(m_selection.bounds.w * m_selection.bounds.h, 1);
}

void RectSelection::redrawPreview(ToolContext &ctx) {
  ctx.preview->clearRGBA(0, 0, 0, 0);

  if (m_selection.empty()) {
    ctx.preview->markDirty();
    return;
  }

  SDL_Surface *preview = ctx.preview->getSurface();

  int offsetX = (int)m_selection.offset.x;
  int offsetY = (int)m_selection.offset.y;

  if (m_selection.pixels) {
    auto *src = static_cast<Uint32 *>(m_selection.pixels->pixels);
    int srcPitch = m_selection.pixels->pitch / sizeof(Uint32);

    for (int y = 0; y < m_selection.bounds.h; ++y) {
      for (int x = 0; x < m_selection.bounds.w; ++x) {
        int dx = m_selection.bounds.x + x + offsetX;
        int dy = m_selection.bounds.y + y + offsetY;

        Rasterizer::drawPixel(preview, dx, dy, src[y * srcPitch + x]);
      }
    }
  }

  SDL_Rect b = m_selection.bounds;
  b.x += offsetX;
  b.y += offsetY;

  Rasterizer::drawRectStroke(preview, {(float)b.x, (float)b.y},
                             {(float)(b.x + b.w), (float)(b.y + b.h)},
                             0xFFFFFFFF, 1);

  ctx.preview->markDirty();
}

void RectSelection::onMouseDown(vec2 pos, ToolContext &ctx) {
  if (m_state == SelectionState::Selected) {
    if (containsPoint(pos)) {
      m_state = SelectionState::Dragging;
      m_dragLast = pos;
      return;
    }

    // Click outside → commit any move then start a new selection.
    if (auto cmd = commitSelection(ctx)) {
      ctx.commandManager->pushCommand(std::move(cmd), "Move Selection");
    }

    clearSelection();

    ctx.preview->clearRGBA(0, 0, 0, 0);
    ctx.preview->markDirty();
  }

  m_state = SelectionState::Drawing;

  m_start = pos;
  m_current = pos;

  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->markDirty();
}

void RectSelection::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  if (m_state == SelectionState::Drawing) {
    m_current = pos;

    ctx.preview->clearRGBA(0, 0, 0, 0);

    Rasterizer::drawRectStroke(ctx.preview->getSurface(), m_start, m_current,
                               ctx.fgColor, 1);

    ctx.preview->markDirty();
  } else if (m_state == SelectionState::Dragging) {

    vec2 delta = pos - m_dragLast;

    m_selection.offset += delta;
    m_dragLast = pos;

    int cw = ctx.canvas->getWidth();
    int ch = ctx.canvas->getHeight();

    m_selection.offset.x =
        std::clamp(m_selection.offset.x, (float)-m_selection.bounds.x,
                   (float)(cw - m_selection.bounds.x - m_selection.bounds.w));

    m_selection.offset.y =
        std::clamp(m_selection.offset.y, (float)-m_selection.bounds.y,
                   (float)(ch - m_selection.bounds.y - m_selection.bounds.h));

    redrawPreview(ctx);
  }
}

std::unique_ptr<Command> RectSelection::onMouseUp(vec2 pos, ToolContext &ctx) {

  if (!ctx.interaction->active)
    return nullptr;

  if (m_state == SelectionState::Drawing) {

    int left = std::min((int)m_start.x, (int)pos.x);
    int top = std::min((int)m_start.y, (int)pos.y);

    int right = std::max((int)m_start.x, (int)pos.x);
    int bottom = std::max((int)m_start.y, (int)pos.y);

    m_selection.bounds = {left, top, right - left + 1, bottom - top + 1};

    if (m_selection.bounds.w <= 1 || m_selection.bounds.h <= 1) {

      clearSelection();

      ctx.preview->clearRGBA(0, 0, 0, 0);
      ctx.preview->markDirty();

      return nullptr;
    }

    buildMask();

    copyFromCanvas(ctx.canvas->getSurface());

    m_selection.offset = {0.f, 0.f};

    m_state = SelectionState::Selected;

    redrawPreview(ctx);
  } else if (m_state == SelectionState::Dragging) {
    m_state = SelectionState::Selected;
    redrawPreview(ctx);
  }

  return nullptr;
}

void RectSelection::deactivate() {
  clearSelection();
  m_state = SelectionState::Idle;
}
