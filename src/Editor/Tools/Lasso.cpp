#include "Lasso.h"

#include "Editor/Commands/Command.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"

#include "Document/PreviewLayer.h"
#include <climits>
#include <cmath>

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────

static float dist(vec2 a, vec2 b) {
  float dx = b.x - a.x, dy = b.y - a.y;
  return std::sqrt(dx * dx + dy * dy);
}

bool Lasso::pointInPolygon(int x, int y) const {
  bool inside = false;
  size_t n = m_points.size();
  if (n < 3)
    return false;
  for (size_t i = 0, j = n - 1; i < n; j = i++) {
    const vec2 &a = m_points[i];
    const vec2 &b = m_points[j];
    if (a.y == b.y)
      continue;
    double ix =
        (double)(b.x - a.x) * (double)(y - a.y) / (double)(b.y - a.y) + a.x;
    if (((a.y > y) != (b.y > y)) && ((double)x < ix))
      inside = !inside;
  }
  return inside;
}

bool Lasso::containsPoint(vec2 pos) const {
  if (m_selection.empty())
    return false;
  int lx = (int)pos.x - m_selection.bounds.x - (int)m_selection.offset.x;
  int ly = (int)pos.y - m_selection.bounds.y - (int)m_selection.offset.y;
  if (lx < 0 || ly < 0 || lx >= m_selection.bounds.w ||
      ly >= m_selection.bounds.h)
    return false;
  return m_selection.mask[ly * m_selection.bounds.w + lx] != 0;
}

void Lasso::expandBounds(vec2 p) {
  m_bounds.x = std::min(m_bounds.x, (int)p.x);
  m_bounds.y = std::min(m_bounds.y, (int)p.y);
  int maxX = std::max(m_bounds.x + m_bounds.w - 1, (int)p.x);
  int maxY = std::max(m_bounds.y + m_bounds.h - 1, (int)p.y);
  m_bounds.w = maxX - m_bounds.x + 1;
  m_bounds.h = maxY - m_bounds.y + 1;
}
/*
SDL_Rect Lasso::computeBounds() const {
  int minX = INT_MAX, minY = INT_MAX, maxX = INT_MIN, maxY = INT_MIN;
  for (const auto &p : m_points) {
    minX = std::min(minX, (int)p.x);
    minY = std::min(minY, (int)p.y);
    maxX = std::max(maxX, (int)p.x);
    maxY = std::max(maxY, (int)p.y);
  }
  if (minX > maxX || minY > maxY)
    return {0, 0, 0, 0};
  return {minX, minY, maxX - minX + 1, maxY - minY + 1};
}
*/
// ─────────────────────────────────────────────────────────────
//  Mask
// ─────────────────────────────────────────────────────────────

void Lasso::buildMask() {
  m_selection.bounds = m_bounds;
  m_selection.mask.assign(m_bounds.w * m_bounds.h, 0);
  for (int y = 0; y < m_bounds.h; ++y)
    for (int x = 0; x < m_bounds.w; ++x)
      if (pointInPolygon(m_bounds.x + x, m_bounds.y + y))
        m_selection.mask[y * m_bounds.w + x] = 1;
}

// ─────────────────────────────────────────────────────────────
//  Preview
// ─────────────────────────────────────────────────────────────

void Lasso::redrawPreview(ToolContext &ctx) {
  ctx.preview->clearRGBA(0, 0, 0, 0);
  if (m_selection.empty()) {
    ctx.preview->markDirty();
    return;
  }

  SDL_Surface *preview = ctx.preview->getSurface();
  int offsetX = (int)m_selection.offset.x;
  int offsetY = (int)m_selection.offset.y;

  // Draw floating pixels at their current (possibly offset) position.
  if (m_selection.pixels) {
    auto *src = static_cast<Uint32 *>(m_selection.pixels->pixels);
    int srcPitch = m_selection.pixels->pitch / sizeof(Uint32);

    for (int y = 0; y < m_selection.bounds.h; ++y) {
      for (int x = 0; x < m_selection.bounds.w; ++x) {
        if (!m_selection.mask[y * m_selection.bounds.w + x])
          continue;
        int dx = m_selection.bounds.x + x + offsetX;
        int dy = m_selection.bounds.y + y + offsetY;
        Rasterizer::drawPixel(preview, dx, dy, src[y * srcPitch + x]);
      }
    }
  }

  // White bounding-box outline so the user can see the selection boundary.
  SDL_Rect b = m_selection.bounds;
  b.x += offsetX;
  b.y += offsetY;
  Rasterizer::drawRectStroke(preview, {(float)b.x, (float)b.y},
                             {(float)(b.x + b.w), (float)(b.y + b.h)},
                             0xFFFFFFFF, 1);

  ctx.preview->markDirty();
}

// ─────────────────────────────────────────────────────────────
//  Tool interface
// ─────────────────────────────────────────────────────────────

void Lasso::onMouseDown(vec2 pos, ToolContext &ctx) {
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
  ctx.preview->clearRGBA(0, 0, 0, 0);
  m_points.clear();
  m_start = pos;
  m_last = pos;
  m_bounds = {(int)pos.x, (int)pos.y, 1, 1};
  m_points.push_back(pos);
  Logger::debug("Lasso START");
}

void Lasso::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  if (m_state == SelectionState::Drawing) {
    if (dist(pos, m_last) < 1.0f)
      return;

    expandBounds(pos);
    m_points.push_back(pos);

    Rasterizer::bresenham(m_last, pos, ctx.preview->getSurface(), ctx.fgColor,
                          1, false);

    SDL_Rect dirty{std::min((int)m_last.x, (int)pos.x) - 1,
                   std::min((int)m_last.y, (int)pos.y) - 1,
                   std::abs((int)(pos.x - m_last.x)) + 3,
                   std::abs((int)(pos.y - m_last.y)) + 3};
    ctx.preview->invalidateRect(dirty);
    m_last = pos;
  } else if (m_state == SelectionState::Dragging) {
    vec2 delta = pos - m_dragLast;
    m_selection.offset.x += delta.x;
    m_selection.offset.y += delta.y;
    m_dragLast = pos;

    // Clamp so selection stays at least 1px inside canvas
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

std::unique_ptr<Command> Lasso::onMouseUp(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return nullptr;

  if (m_state == SelectionState::Drawing) {
    // Close the loop.
    m_points.push_back(m_start);
    Rasterizer::bresenham(pos, m_start, ctx.preview->getSurface(), ctx.fgColor,
                          1, false);

    // m_bounds = computeBounds();
    if (m_bounds.w <= 0 || m_bounds.h <= 0) {
      m_state = SelectionState::Idle;
      return nullptr;
    }

    buildMask();

    if (m_selection.empty()) {
      m_state = SelectionState::Idle;
      ctx.preview->clearRGBA(0, 0, 0, 0);
      ctx.preview->markDirty();
      return nullptr;
    }

    m_selection.offset = {0.f, 0.f};
    copyFromCanvas(ctx.canvas->getSurface());
    m_state = SelectionState::Selected;
    redrawPreview(ctx);

    Logger::debug(std::format("Lasso: {} points, bounds {}x{}", m_points.size(),
                              m_bounds.w, m_bounds.h));

  } else if (m_state == SelectionState::Dragging) {
    m_state = SelectionState::Selected;
    redrawPreview(ctx);
  }

  return nullptr;
}

void Lasso::deactivate() {
  // Can't commit to canvas here (no ToolContext), just discard.
  clearSelection();
  m_points.clear();
  m_state = SelectionState::Idle;
}
