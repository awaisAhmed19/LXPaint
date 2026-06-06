#pragma once

#include "UI/Styling/UIStyle.h"
#include <imgui.h>

namespace UIDrawHelpers {

inline void drawRaisedBorder(ImDrawList *draw, ImVec2 min, ImVec2 max) {
  draw->AddLine(min, {max.x, min.y}, UIStyle::colorHighlight());
  draw->AddLine(min, {min.x, max.y}, UIStyle::colorHighlight());
  draw->AddLine({min.x, max.y - 1}, {max.x, max.y - 1}, UIStyle::colorBorder());
  draw->AddLine({max.x - 1, min.y}, {max.x - 1, max.y}, UIStyle::colorBorder());
}
inline void drawSunkenBorder(ImDrawList *draw, ImVec2 min, ImVec2 max) {
  draw->AddLine(min, {max.x, min.y}, UIStyle::colorBorder());
  draw->AddLine(min, {min.x, max.y}, UIStyle::colorBorder());
  draw->AddLine({min.x, max.y - 1}, {max.x, max.y - 1},
                UIStyle::colorHighlight());
  draw->AddLine({max.x - 1, min.y}, {max.x - 1, max.y},
                UIStyle::colorHighlight());
}

inline void drawFlatBox(ImDrawList *draw, ImVec2 min, ImVec2 max, ImU32 fill,
                        ImU32 border) {
  draw->AddRectFilled(min, max, fill);
  draw->AddRect(min, max, border);
}

inline void drawVerticalSeparator(ImDrawList *draw, float x, float yTop,
                                  float yBottom) {
  float yT = yTop + UIStyle::SeparatorMarginY;
  float yB = yBottom - UIStyle::SeparatorMarginY;

  draw->AddLine({x, yT}, {x, yB}, UIStyle::colorBorder());
  draw->AddLine({x + 1, yT}, {x + 1, yB}, UIStyle::colorHighlight());
}
} // namespace UIDrawHelpers
