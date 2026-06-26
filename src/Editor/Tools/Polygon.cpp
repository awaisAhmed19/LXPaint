#include "Polygon.h"

#include "Editor/Interaction/ToolContext.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Editor/ToolSettings.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
#include <algorithm>
#include <cmath>

#include "Document/PreviewLayer.h"
namespace {
constexpr int kBoundsPad = 4; // safety margin so stroke width / handles
                              // never get clipped at the edge of the
                              // invalidated rect

float dist(vec2 a, vec2 b) {
  float dx = b.x - a.x, dy = b.y - a.y;
  return std::sqrt(dx * dx + dy * dy);
}

// Closing-hitbox indicator: a small circle outline around the first vertex,
// drawn whenever the cursor is within range, so the user can see where the
// close-trigger zone is — same spirit as Lasso's selection outline.
void drawHitboxIndicator(SDL_Surface *surf, vec2 center, float radius,
                         uint32_t color) {
  Rasterizer::drawEllipse(surf, (int)center.x, (int)center.y, (int)radius,
                          (int)radius, color);
}
} // namespace

// ─────────────────────────────────────────────────────────────
//  Bounds tracking
// ─────────────────────────────────────────────────────────────

void Polygon::expandBounds(vec2 p) {
  if (m_points.empty()) {
    m_bounds = {(int)p.x, (int)p.y, 1, 1};
    return;
  }

  int minX = std::min(m_bounds.x, (int)p.x);
  int minY = std::min(m_bounds.y, (int)p.y);
  int maxX = std::max(m_bounds.x + m_bounds.w - 1, (int)p.x);
  int maxY = std::max(m_bounds.y + m_bounds.h - 1, (int)p.y);

  m_bounds = {minX, minY, maxX - minX + 1, maxY - minY + 1};
}

SDL_Rect Polygon::computeFullBounds(int padding, int maxW, int maxH) const {
  // Bounds of all committed vertices plus the live cursor point (the
  // rubber-band edge and closing-hitbox circle can extend past the last
  // committed vertex), padded and clamped to the surface.
  SDL_Rect b = m_bounds;

  int minX = std::min(b.x, (int)m_cursor.x);
  int minY = std::min(b.y, (int)m_cursor.y);
  int maxX = std::max(b.x + b.w - 1, (int)m_cursor.x);
  int maxY = std::max(b.y + b.h - 1, (int)m_cursor.y);

  // The closing hitbox circle is drawn around the first vertex with radius
  // kCloseRadius — include that in the padding so it's never clipped.
  int pad = padding + (int)kCloseRadius;

  int x = std::max(0, minX - pad);
  int y = std::max(0, minY - pad);
  int x2 = std::min(maxW - 1, maxX + pad);
  int y2 = std::min(maxH - 1, maxY + pad);

  return SDL_Rect{x, y, x2 - x + 1, y2 - y + 1};
}

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────

bool Polygon::nearStart(vec2 pos) const {
  if (m_points.empty())
    return false;
  return dist(pos, m_points.front()) <= kCloseRadius;
}

Polygon::FillMode Polygon::currentFillMode(ToolContext &ctx) const {
  // Reuses ToolSettings::brushShape as the fill-mode selector, exactly as
  // Editor.h documents it being reused for Rectangle/Ellipse/Polygon/
  // RoundedRectangle ("same field, different meaning depending on active
  // tool"). Mapping: Round -> Outline, Square -> Filled (foreground),
  // ForwardSlash -> Opaque (background). BackSlash is unused for Polygon.
  switch (ctx.settings->brushShape) {
  case ToolSettings::BrushShape::Round:
    return FillMode::Outline;
  case ToolSettings::BrushShape::Square:
    return FillMode::Filled;
  case ToolSettings::BrushShape::ForwardSlash:
    return FillMode::Opaque;
  default:
    return FillMode::Outline;
  }
}

void Polygon::reset() {
  m_state = State::Idle;
  m_pendingClose = false;
  m_points.clear();
  m_bounds = {0, 0, 0, 0};
  m_command.reset();
}

// ─────────────────────────────────────────────────────────────
//  Preview
// ─────────────────────────────────────────────────────────────

void Polygon::redrawPreview(ToolContext &ctx) {
  ctx.preview->clearRGBA(0, 0, 0, 0);

  if (m_state != State::Drawing || m_points.empty()) {
    ctx.preview->markDirty();
    return;
  }

  SDL_Surface *surf = ctx.preview->getSurface();

  // Preview fill: only meaningful once there are enough vertices to form a
  // polygon, and only for the two modes that actually fill on close. The
  // live cursor is included as an implicit closing point so the filled
  // preview tracks the rubber-band edge instead of only the already-
  // committed (still-open) vertex chain — otherwise the fill would lag one
  // edge behind what the user is about to commit.
  if (m_points.size() >= kMinVertices) {
    FillMode mode = currentFillMode(ctx);
    if (mode == FillMode::Filled || mode == FillMode::Opaque) {
      std::vector<vec2> previewLoop = m_points;
      previewLoop.push_back(m_cursor);
      uint32_t fillColor =
          (mode == FillMode::Filled) ? ctx.fgColor : ctx.bgColor;
      Rasterizer::fillPolygon(surf, previewLoop, fillColor);
    }
  }

  // Existing committed edges. Drawn after the fill so edges stay on top —
  // same fill-then-stroke order used at commit time (see onMouseUp).
  for (size_t i = 0; i + 1 < m_points.size(); ++i) {
    Rasterizer::bresenham(m_points[i], m_points[i + 1], surf, ctx.fgColor, 1,
                          false);
  }

  // Rubber-band edge from the last committed vertex to the cursor.
  Rasterizer::bresenham(m_points.back(), m_cursor, surf, ctx.fgColor, 1, false);

  // Closing-hitbox indicator around the first vertex once enough vertices
  // exist to legally close, so the user can see the trigger zone.
  if (m_points.size() >= kMinVertices) {
    uint32_t hitColor = nearStart(m_cursor) ? 0xFF00FF00u : 0xFF808080u;
    drawHitboxIndicator(surf, m_points.front(), kCloseRadius, hitColor);
  }

  SDL_Rect dirty = computeFullBounds(kBoundsPad, ctx.preview->getWidth(),
                                     ctx.preview->getHeight());

  ctx.preview->invalidateRect(dirty);
}

// ─────────────────────────────────────────────────────────────
//  Tool interface
// ─────────────────────────────────────────────────────────────

void Polygon::onMouseDown(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Polygon canvas missing");
  LX_ASSERT(ctx.interaction != nullptr, "Polygon interaction missing");

  if (m_state == State::Idle) {
    Logger::debug("Polygon: first vertex");
    reset();
    m_state = State::Drawing;
    m_points.push_back(pos);
    m_cursor = pos;
    expandBounds(pos);
    redrawPreview(ctx);
    return;
  }

  // Already drawing — is this click the closing click?
  if (nearStart(pos) && m_points.size() >= kMinVertices) {
    Logger::debug("Polygon: closing click");
    m_pendingClose = true;
    m_cursor = m_points.front(); // snap exactly onto the start vertex
    return;
  }

  // Otherwise it's a regular new vertex.
  m_points.push_back(pos);
  m_cursor = pos;
  expandBounds(pos);
  redrawPreview(ctx);
}

void Polygon::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (m_state != State::Drawing)
    return;

  m_cursor = pos;
  redrawPreview(ctx);
}

std::unique_ptr<Command> Polygon::onMouseUp(vec2 pos, ToolContext &ctx) {
  if (!m_pendingClose) {
    // Intermediate vertex placement — no canvas mutation yet, no undo entry.
    return nullptr;
  }

  Logger::debug("Polygon: committing to canvas");

  SDL_Surface *canvasSurf = ctx.canvas->getSurface();

  SDL_Rect affected =
      computeFullBounds(kBoundsPad, canvasSurf->w, canvasSurf->h);

  // Capture BEFORE state — canvas is still unmodified at this point.
  m_command = std::make_unique<SnapshotCommand>(canvasSurf, affected);

  // Erase the preview overlay; the shape now lives on the canvas.
  ctx.preview->clearRGBA(0, 0, 0, 0);
  ctx.preview->invalidateRect(affected);

  FillMode mode = currentFillMode(ctx);

  if (mode == FillMode::Filled) {
    Rasterizer::fillPolygon(canvasSurf, m_points, ctx.fgColor);
  } else if (mode == FillMode::Opaque) {
    Rasterizer::fillPolygon(canvasSurf, m_points, ctx.bgColor);
  }

  // Edges are always drawn, on top of any fill, in all three modes.
  for (size_t i = 0; i + 1 < m_points.size(); ++i) {
    Rasterizer::bresenham(m_points[i], m_points[i + 1], canvasSurf, ctx.fgColor,
                          1, false);
  }
  // Closing edge back to the first vertex.
  Rasterizer::bresenham(m_points.back(), m_points.front(), canvasSurf,
                        ctx.fgColor, 1, false);

  ctx.canvas->invalidateRect(affected);

  // Capture AFTER state for redo.
  m_command->captureAfter(canvasSurf);

  reset();

  return std::move(m_command);
}

bool Polygon::onKeyDown(SDL_Scancode scancode, ToolContext &ctx) {
  if (scancode == SDL_SCANCODE_ESCAPE && m_state == State::Drawing) {
    Logger::debug("Polygon: cancelled via Escape");

    // Clear the preview overlay; canvas was never touched, so there is
    // nothing to undo and no SnapshotCommand is created.
    ctx.preview->clearRGBA(0, 0, 0, 0);

    SDL_Rect dirty = computeFullBounds(kBoundsPad, ctx.preview->getWidth(),
                                       ctx.preview->getHeight());
    ctx.preview->invalidateRect(dirty);

    reset();
    return true;
  }

  return false;
}

void Polygon::deactivate() {
  // Switching tools mid-polygon discards it — no ToolContext is available
  // here to clear the preview surface directly, but the next tool's first
  // updatePreview/clearRGBA call (or Editor re-clearing on tool switch)
  // will cover it. No history is created either way.
  Logger::debug("Polygon: deactivated — discarding in-progress polygon");
  reset();
  BaseTool::deactivate();
}
