#include "CurveLine.h"

#include "Editor/Editor.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Logger.h"
#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────────────────────

static constexpr int BEZIER_STEPS = 100; // line segments to approximate curve
static constexpr int MARKER_HALF = 3; // half-size of square handle markers (px)
static constexpr int BOUNDS_PAD = 4;  // extra safety margin on affected rect
static constexpr int DASH_ON = 5;     // guide-line dash length (px)
static constexpr int DASH_OFF = 4;    // guide-line gap length (px)

// Colours for handle markers (ARGB)
static constexpr uint32_t COLOR_GUIDE = 0xFF808080;  // grey dashed guide lines
static constexpr uint32_t COLOR_ANCHOR = 0xFFFFFFFF; // white  endpoint handles
static constexpr uint32_t COLOR_CP1 = 0xFF0000FF;    // blue   CP1 handle
static constexpr uint32_t COLOR_CP2 = 0xFFFF0000;    // red    CP2 handle
static constexpr uint32_t COLOR_BORDER = 0xFF000000; // black  handle border

// ─────────────────────────────────────────────────────────────
//  Bezier math
// ─────────────────────────────────────────────────────────────

static vec2 evalCubicBezier(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
  const float u = 1.0f - t;
  const float uu = u * u;
  const float uuu = uu * u;
  const float tt = t * t;
  const float ttt = tt * t;
  return {uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
          uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y +
              ttt * p3.y};
}

// ─────────────────────────────────────────────────────────────
//  Private draw helpers
// ─────────────────────────────────────────────────────────────

void CurveLine::drawBezierToSurface(SDL_Surface *surf, uint32_t color,
                                    int lw) const {
  vec2 prev = m_p0;
  for (int i = 1; i <= BEZIER_STEPS; ++i) {
    const float t = static_cast<float>(i) / BEZIER_STEPS;
    const vec2 pt = evalCubicBezier(m_p0, m_p1, m_p2, m_p3, t);
    Rasterizer::bresenham(prev, pt, surf, color, lw, false);
    prev = pt;
  }
}

void CurveLine::drawDashedLine(SDL_Surface *surf, vec2 a, vec2 b,
                               uint32_t color) const {
  const float dx = b.x - a.x;
  const float dy = b.y - a.y;
  const float len = std::sqrt(dx * dx + dy * dy);
  if (len < 1.0f)
    return;

  const float nx = dx / len;
  const float ny = dy / len;
  float t = 0.0f;
  bool on = true;

  while (t < len) {
    const float seg =
        on ? static_cast<float>(DASH_ON) : static_cast<float>(DASH_OFF);
    const float t1 = std::min(t + seg, len);

    if (on) {
      const vec2 pa = {a.x + nx * t, a.y + ny * t};
      const vec2 pb = {a.x + nx * t1, a.y + ny * t1};
      Rasterizer::bresenham(pa, pb, surf, color, 1, false);
    }

    t += seg;
    on = !on;
  }
}

void CurveLine::drawSquareMarker(SDL_Surface *surf, vec2 pos, uint32_t fill,
                                 uint32_t border) const {
  const int cx = static_cast<int>(pos.x);
  const int cy = static_cast<int>(pos.y);

  // Solid interior
  for (int dy = -MARKER_HALF; dy <= MARKER_HALF; ++dy)
    for (int dx = -MARKER_HALF; dx <= MARKER_HALF; ++dx)
      Rasterizer::drawPixel(surf, cx + dx, cy + dy, fill);

  // One-pixel border
  for (int d = -MARKER_HALF; d <= MARKER_HALF; ++d) {
    Rasterizer::drawPixel(surf, cx + d, cy - MARKER_HALF, border);
    Rasterizer::drawPixel(surf, cx + d, cy + MARKER_HALF, border);
    Rasterizer::drawPixel(surf, cx - MARKER_HALF, cy + d, border);
    Rasterizer::drawPixel(surf, cx + MARKER_HALF, cy + d, border);
  }
}

SDL_Rect CurveLine::computeAffectedRect(int lw, int surfW, int surfH) const {
  // Cubic bezier lies within the convex hull of its control polygon,
  // so the bounding box of all four points covers the curve entirely.
  const int minX =
      std::min({(int)m_p0.x, (int)m_p1.x, (int)m_p2.x, (int)m_p3.x});
  const int minY =
      std::min({(int)m_p0.y, (int)m_p1.y, (int)m_p2.y, (int)m_p3.y});
  const int maxX =
      std::max({(int)m_p0.x, (int)m_p1.x, (int)m_p2.x, (int)m_p3.x});
  const int maxY =
      std::max({(int)m_p0.y, (int)m_p1.y, (int)m_p2.y, (int)m_p3.y});

  // pad = line-radius + marker extent + safety margin
  const int pad = lw + MARKER_HALF + BOUNDS_PAD;

  const int x = std::max(0, minX - pad);
  const int y = std::max(0, minY - pad);
  const int x2 = std::min(surfW - 1, maxX + pad);
  const int y2 = std::min(surfH - 1, maxY + pad);

  return SDL_Rect{x, y, x2 - x + 1, y2 - y + 1};
}

void CurveLine::setDefaultControlPoints() {
  // Place P1 at 1/3 and P2 at 2/3 along the P0→P3 segment so the
  // initial bezier degenerates to the straight line the user just drew.
  m_p1 = {m_p0.x + (m_p3.x - m_p0.x) / 3.0f, m_p0.y + (m_p3.y - m_p0.y) / 3.0f};
  m_p2 = {m_p0.x + 2.0f * (m_p3.x - m_p0.x) / 3.0f,
          m_p0.y + 2.0f * (m_p3.y - m_p0.y) / 3.0f};
}

// ─────────────────────────────────────────────────────────────
//  Preview composition
// ─────────────────────────────────────────────────────────────

void CurveLine::updatePreview(ToolContext &ctx, int lw) {
  ctx.preview->clearRGBA(0, 0, 0, 0);

  SDL_Surface *surf = ctx.preview->getSurface();
  const int surfW = surf->w;
  const int surfH = surf->h;

  switch (m_phase) {

  case BezierPhase::ENDPOINTS:
    // Cycle 1: straight-line stand-in while endpoints are being placed
    Rasterizer::bresenham(m_p0, m_p3, surf, ctx.fgColor, lw, false);
    drawSquareMarker(surf, m_p0, COLOR_ANCHOR, COLOR_BORDER);
    drawSquareMarker(surf, m_p3, COLOR_ANCHOR, COLOR_BORDER);
    break;

  case BezierPhase::CP1:
    // Cycle 2: live curve + guide from P0 → CP1
    drawBezierToSurface(surf, ctx.fgColor, lw);
    drawDashedLine(surf, m_p0, m_p1, COLOR_GUIDE);
    drawSquareMarker(surf, m_p0, COLOR_ANCHOR, COLOR_BORDER);
    drawSquareMarker(surf, m_p3, COLOR_ANCHOR, COLOR_BORDER);
    drawSquareMarker(surf, m_p1, COLOR_CP1, COLOR_BORDER);
    break;

  case BezierPhase::CP2:
    // Cycle 3: live curve + both guides
    drawBezierToSurface(surf, ctx.fgColor, lw);
    drawDashedLine(surf, m_p0, m_p1, COLOR_GUIDE);
    drawDashedLine(surf, m_p3, m_p2, COLOR_GUIDE);
    drawSquareMarker(surf, m_p0, COLOR_ANCHOR, COLOR_BORDER);
    drawSquareMarker(surf, m_p3, COLOR_ANCHOR, COLOR_BORDER);
    drawSquareMarker(surf, m_p1, COLOR_CP1, COLOR_BORDER);
    drawSquareMarker(surf, m_p2, COLOR_CP2, COLOR_BORDER);
    break;

  default:
    break;
  }

  m_affected = computeAffectedRect(lw, surfW, surfH);
  ctx.preview->invalidateRect(m_affected);
}

// ─────────────────────────────────────────────────────────────
//  Tool interface
// ─────────────────────────────────────────────────────────────

void CurveLine::onMouseDown(vec2 pos, ToolContext &ctx) {
  const int lw = std::max(1, static_cast<int>(ctx.settings->lineWidth));

  switch (m_phase) {

  case BezierPhase::IDLE:
    Logger::debug("CurveLine: cycle 1 — setting endpoints");
    m_p0 = m_p1 = m_p2 = m_p3 = pos;
    m_phase = BezierPhase::ENDPOINTS;
    break;

  case BezierPhase::CP1:
    // User begins dragging CP1 — snap the handle to the click position
    // and show immediate feedback before the first mousemove fires.
    Logger::debug("CurveLine: cycle 2 — adjusting CP1");
    m_p1 = pos;
    updatePreview(ctx, lw);
    break;

  case BezierPhase::CP2:
    Logger::debug("CurveLine: cycle 3 — adjusting CP2");
    m_p2 = pos;
    updatePreview(ctx, lw);
    break;

  default:
    break;
  }
}

void CurveLine::onMouseMove(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return;

  const int lw = std::max(1, static_cast<int>(ctx.settings->lineWidth));

  switch (m_phase) {

  case BezierPhase::ENDPOINTS:
    m_p3 = pos;
    setDefaultControlPoints();
    break;

  case BezierPhase::CP1:
    m_p1 = pos;
    break;

  case BezierPhase::CP2:
    m_p2 = pos;
    break;

  default:
    return;
  }

  updatePreview(ctx, lw);
}

std::unique_ptr<Command> CurveLine::onMouseUp(vec2 pos, ToolContext &ctx) {
  if (!ctx.interaction->active)
    return nullptr;

  const int lw = std::max(1, static_cast<int>(ctx.settings->lineWidth));

  switch (m_phase) {

  // ── Cycle 1 complete: lock endpoints, wait for CP1 drag ─────────────
  case BezierPhase::ENDPOINTS:
    m_p3 = pos;
    setDefaultControlPoints();
    m_phase = BezierPhase::CP1;
    Logger::debug("CurveLine: endpoints locked — awaiting CP1");
    // Show the degenerate (straight) bezier with CP1 marker so the
    // user knows what to drag next.
    updatePreview(ctx, lw);
    return nullptr;

  // ── Cycle 2 complete: lock CP1, wait for CP2 drag ───────────────────
  case BezierPhase::CP1:
    m_p1 = pos;
    m_phase = BezierPhase::CP2;
    Logger::debug("CurveLine: CP1 locked — awaiting CP2");
    updatePreview(ctx, lw);
    return nullptr;

  // ── Cycle 3 complete: lock CP2, commit curve permanently ────────────
  case BezierPhase::CP2: {
    m_p2 = pos;
    Logger::debug("CurveLine: committing bezier to canvas");

    SDL_Surface *canvasSurf = ctx.canvas->getSurface();

    // Compute final bounds against canvas (may differ from preview bounds
    // only if canvas was resized between cycles, which is guarded elsewhere).
    m_affected = computeAffectedRect(lw, canvasSurf->w, canvasSurf->h);

    // Capture BEFORE state — canvas is still unmodified at this point.
    m_command = std::make_unique<SnapshotCommand>(canvasSurf, m_affected);

    // Erase the preview overlay.
    ctx.preview->clearRGBA(0, 0, 0, 0);
    ctx.preview->invalidateRect(m_affected);

    // Draw the curve permanently.
    drawBezierToSurface(canvasSurf, ctx.fgColor, lw);
    ctx.canvas->invalidateRect(m_affected);

    // Capture AFTER state for redo.
    m_command->captureAfter(canvasSurf);

    m_phase = BezierPhase::IDLE;

    return std::move(m_command);
  }

  default:
    return nullptr;
  }
}

void CurveLine::deactivate() {
  Logger::debug("CurveLine: deactivated — resetting to IDLE");
  // Reset all state so a fresh activation starts cleanly.
  // The preview surface is not cleared here (no ToolContext); it will be
  // cleared by the next tool interaction via updatePreview / clearRGBA.
  m_phase = BezierPhase::IDLE;
  m_p0 = m_p1 = m_p2 = m_p3 = {};
  m_affected = {};
  m_command.reset();
  BaseTool::deactivate();
}
