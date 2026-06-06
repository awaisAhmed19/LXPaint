#pragma once

#include "UI/Styling/UIStyle.h"
#include <imgui.h>

namespace UIDrawHelpers {

/**
 * Draw a raised 3D border (embossed effect)
 * - Top/Left: bright highlight (appears raised)
 * - Bottom/Right: dark border (appears recessed)
 */
inline void drawRaisedBorder(ImDrawList *draw, ImVec2 min, ImVec2 max,
                             float thickness = 1.0f) {
  ImVec2 maxAdjusted = {max.x - thickness, max.y - thickness};

  // Top-left: bright edges (highlight)
  draw->AddLine(min, {max.x, min.y}, UIStyle::colorHighlight(), thickness);
  draw->AddLine(min, {min.x, max.y}, UIStyle::colorHighlight(), thickness);

  // Bottom-right: dark edges (shadow)
  draw->AddLine({min.x, maxAdjusted.y}, {max.x, maxAdjusted.y},
                UIStyle::colorBorder(), thickness);
  draw->AddLine({maxAdjusted.x, min.y}, {maxAdjusted.x, max.y},
                UIStyle::colorBorder(), thickness);
}

/**
 * Draw a sunken 3D border (inset/depressed effect)
 * - Top/Left: dark border (appears recessed)
 * - Bottom/Right: bright highlight (appears raised)
 * Inverse of drawRaisedBorder
 */
inline void drawSunkenBorder(ImDrawList *draw, ImVec2 min, ImVec2 max,
                             float thickness = 1.0f) {
  ImVec2 maxAdjusted = {max.x - thickness, max.y - thickness};

  // Top-left: dark edges (shadow)
  draw->AddLine(min, {max.x, min.y}, UIStyle::colorBorder(), thickness);
  draw->AddLine(min, {min.x, max.y}, UIStyle::colorBorder(), thickness);

  // Bottom-right: bright edges (highlight)
  draw->AddLine({min.x, maxAdjusted.y}, {max.x, maxAdjusted.y},
                UIStyle::colorHighlight(), thickness);
  draw->AddLine({maxAdjusted.x, min.y}, {maxAdjusted.x, max.y},
                UIStyle::colorHighlight(), thickness);
}

/**
 * Draw a flat rectangular border with no 3D effect
 */
inline void drawFlatBorder(ImDrawList *draw, ImVec2 min, ImVec2 max,
                           ImU32 color, float thickness = 1.0f) {
  draw->AddRect(min, max, color, 0.0f, 0, thickness);
}

/**
 * Draw a filled rectangle with a flat border
 * Convenience function combining fill + border
 */
inline void drawBox(ImDrawList *draw, ImVec2 min, ImVec2 max, ImU32 fill,
                    ImU32 border, float borderThickness = 1.0f) {
  draw->AddRectFilled(min, max, fill);
  drawFlatBorder(draw, min, max, border, borderThickness);
}

/**
 * Draw a filled rectangle with a raised 3D border
 */
inline void drawRaisedBox(ImDrawList *draw, ImVec2 min, ImVec2 max, ImU32 fill,
                          float thickness = 1.0f) {
  draw->AddRectFilled(min, max, fill);
  drawRaisedBorder(draw, min, max, thickness);
}

/**
 * Draw a filled rectangle with a sunken 3D border
 */
inline void drawSunkenBox(ImDrawList *draw, ImVec2 min, ImVec2 max, ImU32 fill,
                          float thickness = 1.0f) {
  draw->AddRectFilled(min, max, fill);
  drawSunkenBorder(draw, min, max, thickness);
}

/**
 * Draw a vertical separator line with 3D effect
 * - Left line: dark border
 * - Right line: bright highlight
 * Creates a grooved effect
 */
inline void drawVerticalSeparator(ImDrawList *draw, float x, float yTop,
                                  float yBottom, float thickness = 1.0f) {
  float yT = yTop + UIStyle::SeparatorMarginY;
  float yB = yBottom - UIStyle::SeparatorMarginY;

  // Left edge: dark
  draw->AddLine({x, yT}, {x, yB}, UIStyle::colorBorder(), thickness);
  // Right edge: bright (creates 3D groove effect)
  draw->AddLine({x + thickness, yT}, {x + thickness, yB},
                UIStyle::colorHighlight(), thickness);
}

/**
 * Draw a horizontal separator line with 3D effect
 * - Top line: dark border
 * - Bottom line: bright highlight
 * Creates a grooved effect
 */
inline void drawHorizontalSeparator(ImDrawList *draw, float y, float xLeft,
                                    float xRight, float thickness = 1.0f) {
  float xL = xLeft + UIStyle::SeparatorMarginX;
  float xR = xRight - UIStyle::SeparatorMarginX;

  // Top edge: dark
  draw->AddLine({xL, y}, {xR, y}, UIStyle::colorBorder(), thickness);
  // Bottom edge: bright (creates 3D groove effect)
  draw->AddLine({xL, y + thickness}, {xR, y + thickness},
                UIStyle::colorHighlight(), thickness);
}

/**
 * Draw a flat vertical line separator
 */
inline void drawVerticalLine(ImDrawList *draw, float x, float yTop,
                             float yBottom, ImU32 color,
                             float thickness = 1.0f) {
  draw->AddLine({x, yTop}, {x, yBottom}, color, thickness);
}

/**
 * Draw a flat horizontal line separator
 */
inline void drawHorizontalLine(ImDrawList *draw, float y, float xLeft,
                               float xRight, ImU32 color,
                               float thickness = 1.0f) {
  draw->AddLine({xLeft, y}, {xRight, y}, color, thickness);
}

/**
 * Draw a filled rounded rectangle
 */
inline void drawRoundedBox(ImDrawList *draw, ImVec2 min, ImVec2 max, ImU32 fill,
                           float rounding = 4.0f) {
  draw->AddRectFilled(min, max, fill, rounding);
}

/**
 * Draw a circle
 */
inline void drawCircle(ImDrawList *draw, ImVec2 center, float radius,
                       ImU32 color, float thickness = 1.0f) {
  draw->AddCircle(center, radius, color, 0, thickness);
}

/**
 * Draw a filled circle
 */
inline void drawCircleFilled(ImDrawList *draw, ImVec2 center, float radius,
                             ImU32 color) {
  draw->AddCircleFilled(center, radius, color);
}

/**
 * Draw a triangle (useful for arrows, indicators)
 */
inline void drawTriangle(ImDrawList *draw, ImVec2 p1, ImVec2 p2, ImVec2 p3,
                         ImU32 color, float thickness = 1.0f) {
  draw->AddTriangle(p1, p2, p3, color, thickness);
}

/**
 * Draw a filled triangle
 */
inline void drawTriangleFilled(ImDrawList *draw, ImVec2 p1, ImVec2 p2,
                               ImVec2 p3, ImU32 color) {
  draw->AddTriangleFilled(p1, p2, p3, color);
}

// ─────────────────────────────────────────────────────────────
// UTILITY FUNCTIONS
// ─────────────────────────────────────────────────────────────

/**
 * Get the center point of a rectangle
 */
inline ImVec2 getRectCenter(ImVec2 min, ImVec2 max) {
  return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f};
}

/**
 * Get the size of a rectangle
 */
inline ImVec2 getRectSize(ImVec2 min, ImVec2 max) {
  return {max.x - min.x, max.y - min.y};
}

/**
 * Expand a rectangle by a given amount
 */
inline void expandRect(ImVec2 &min, ImVec2 &max, float amount) {
  min.x -= amount;
  min.y -= amount;
  max.x += amount;
  max.y += amount;
}

/**
 * Shrink a rectangle by a given amount
 */
inline void shrinkRect(ImVec2 &min, ImVec2 &max, float amount) {
  min.x += amount;
  min.y += amount;
  max.x -= amount;
  max.y -= amount;
}

/**
 * Check if a point is inside a rectangle
 */
inline bool pointInRect(ImVec2 point, ImVec2 min, ImVec2 max) {
  return point.x >= min.x && point.x <= max.x && point.y >= min.y &&
         point.y <= max.y;
}

} // namespace UIDrawHelpers
