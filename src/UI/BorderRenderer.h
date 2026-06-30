#pragma once
#include "Theme.h"
#include "imgui.h"

// ─────────────────────────────────────────────────────────────────────────
//  UI::BorderRenderer — the one place that knows how to draw a Win98
//  raised/sunken bevel. Every module that previously hand-rolled
//  raisedBorder()/sunkenBorder() (Toolbar, Footer, Ribbon, ColorPalette,
//  Dropdown, Dialog...) should call into this instead.
// ─────────────────────────────────────────────────────────────────────────
namespace UI {

class BorderRenderer {
public:
  static void Raised(ImDrawList *dl, ImVec2 min, ImVec2 max,
                     float thickness = 1.f) {
    dl->AddLine(min, {max.x, min.y}, Theme::BorderHi, thickness);
    dl->AddLine(min, {min.x, max.y}, Theme::BorderHi, thickness);
    dl->AddLine({min.x, max.y}, max, Theme::Black, thickness);
    dl->AddLine({max.x, min.y}, max, Theme::Black, thickness);
  }

  static void Sunken(ImDrawList *dl, ImVec2 min, ImVec2 max,
                     float thickness = 1.f) {
    dl->AddLine(min, {max.x, min.y}, Theme::Black, thickness);
    dl->AddLine(min, {min.x, max.y}, Theme::Black, thickness);
    dl->AddLine({min.x, max.y}, max, Theme::BorderHi, thickness);
    dl->AddLine({max.x, min.y}, max, Theme::BorderHi, thickness);
  }

  // Double-bevel variant used by the outer Dialog/Dropdown frame (1px
  // black outline + 1px inset raised/sunken bevel).
  static void RaisedDouble(ImDrawList *dl, ImVec2 min, ImVec2 max) {
    dl->AddRect(min, max, Theme::BorderOuter, 0.f, 0, 1.f);
    Raised(dl, {min.x + 1, min.y + 1}, {max.x - 1, max.y - 1});
  }

  static void SunkenDouble(ImDrawList *dl, ImVec2 min, ImVec2 max) {
    dl->AddRect(min, max, Theme::BorderOuter, 0.f, 0, 1.f);
    Sunken(dl, {min.x + 1, min.y + 1}, {max.x - 1, max.y - 1});
  }

  static void Outline(ImDrawList *dl, ImVec2 min, ImVec2 max,
                      ImU32 color = Theme::BorderOuter, float thickness = 1.f) {
    dl->AddRect(min, max, color, 0.f, 0, thickness);
  }
};

} // namespace UI
