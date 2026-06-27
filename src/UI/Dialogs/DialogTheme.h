#pragma once
#include "imgui.h"

// ─────────────────────────────────────────────────────────────────────────────
//  DialogTheme
//
//  All Win95 color constants shared across every dialog.
//  Mirrors the palette already used in Ribbon, Dropdown, Footer, etc.
//  One place to change every dialog's appearance.
// ─────────────────────────────────────────────────────────────────────────────

namespace UI::DialogTheme {

// Panel / window face
constexpr ImU32 BG = IM_COL32(192, 192, 192, 255);

// Title bar
constexpr ImU32 TitleBarBg = IM_COL32(0, 0, 128, 255); // Win95 navy
constexpr ImU32 TitleText = IM_COL32(255, 255, 255, 255);
constexpr ImU32 CloseBtn = IM_COL32(192, 192, 192, 255);

// Text
constexpr ImU32 TextNormal = IM_COL32(0, 0, 0, 255);
constexpr ImU32 TextDisabled = IM_COL32(128, 128, 128, 255);

// Buttons
constexpr ImU32 ButtonFace = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonPress = IM_COL32(160, 160, 160, 255);

// Input fields
constexpr ImU32 FieldBg = IM_COL32(255, 255, 255, 255);
constexpr ImU32 FieldText = IM_COL32(0, 0, 0, 255);
constexpr ImU32 FieldSelect = IM_COL32(0, 0, 128, 255);

// Radio / checkbox
constexpr ImU32 RadioFill = IM_COL32(255, 255, 255, 255);
constexpr ImU32 RadioDot = IM_COL32(0, 0, 0, 255);

// Group box
constexpr ImU32 GroupBoxText = IM_COL32(0, 0, 0, 255);

// Border helpers (same as Ribbon/Dropdown)
constexpr ImU32 BorderHi = IM_COL32(255, 255, 255, 255);
constexpr ImU32 BorderLo = IM_COL32(64, 64, 64, 255);
constexpr ImU32 BorderOuter = IM_COL32(0, 0, 0, 255);
constexpr ImU32 BorderShadow = IM_COL32(128, 128, 128, 255);

// Overlay behind modal dialog
constexpr ImU32 Overlay = IM_COL32(0, 0, 0, 64);

// Geometry
constexpr float TitleBarH = 18.0f;
constexpr float ButtonW = 72.0f;
constexpr float ButtonH = 23.0f;
constexpr float FieldH = 20.0f;
constexpr float RadioR = 5.0f; // radius of radio circle
constexpr float CheckSize = 12.0f;
constexpr float Padding = 10.0f;
constexpr float ButtonGap = 6.0f;
constexpr float ControlGapY = 6.0f;

// ─── Border drawing helpers ───────────────────────────────────────────────

// Classic Win95 raised/sunken panel border (2-pixel)
inline void drawRaised(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  // Outer
  dl->AddLine(min, {max.x, min.y}, BorderHi, 1.f);    // top
  dl->AddLine(min, {min.x, max.y}, BorderHi, 1.f);    // left
  dl->AddLine({min.x, max.y}, max, BorderOuter, 1.f); // bottom
  dl->AddLine({max.x, min.y}, max, BorderOuter, 1.f); // right
  // Inner
  ImVec2 i0 = {min.x + 1, min.y + 1};
  ImVec2 i1 = {max.x - 1, max.y - 1};
  dl->AddLine(i0, {i1.x, i0.y}, BorderHi, 1.f);
  dl->AddLine(i0, {i0.x, i1.y}, BorderHi, 1.f);
  dl->AddLine({i0.x, i1.y}, i1, BorderShadow, 1.f);
  dl->AddLine({i1.x, i0.y}, i1, BorderShadow, 1.f);
}

inline void drawSunken(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, BorderShadow, 1.f);
  dl->AddLine(min, {min.x, max.y}, BorderShadow, 1.f);
  dl->AddLine({min.x, max.y}, max, BorderHi, 1.f);
  dl->AddLine({max.x, min.y}, max, BorderHi, 1.f);
  ImVec2 i0 = {min.x + 1, min.y + 1};
  ImVec2 i1 = {max.x - 1, max.y - 1};
  dl->AddLine(i0, {i1.x, i0.y}, BorderOuter, 1.f);
  dl->AddLine(i0, {i0.x, i1.y}, BorderOuter, 1.f);
  dl->AddLine({i0.x, i1.y}, i1, BorderHi, 1.f);
  dl->AddLine({i1.x, i0.y}, i1, BorderHi, 1.f);
}

// Thin single-line sunken (for text fields)
inline void drawFieldBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, BorderShadow, 1.f);
  dl->AddLine(min, {min.x, max.y}, BorderShadow, 1.f);
  dl->AddLine({min.x, max.y}, max, BorderHi, 1.f);
  dl->AddLine({max.x, min.y}, max, BorderHi, 1.f);
}

// Group box (label cutout on top edge)
inline void drawGroupBox(ImDrawList *dl, ImVec2 min, ImVec2 max,
                         const char *label, float labelOffsetX = 8.f) {
  ImVec2 labelSize = ImGui::CalcTextSize(label);
  float lx0 = min.x + labelOffsetX;
  float lx1 = lx0 + labelSize.x + 4.f;
  float my = min.y + labelSize.y * 0.5f;

  // Top line with gap for label
  dl->AddLine({min.x + 1, my}, {lx0 - 2, my}, BorderShadow, 1.f);
  dl->AddLine({lx1 + 2, my}, {max.x - 1, my}, BorderShadow, 1.f);
  dl->AddLine({min.x + 2, my + 1}, {lx0 - 1, my + 1}, BorderHi, 1.f);
  dl->AddLine({lx1 + 2, my + 1}, {max.x, my + 1}, BorderHi, 1.f);

  // Left / bottom / right
  dl->AddLine({min.x, my}, {min.x, max.y}, BorderShadow, 1.f);
  dl->AddLine({min.x + 1, my + 1}, {min.x + 1, max.y - 1}, BorderHi, 1.f);
  dl->AddLine({min.x, max.y}, {max.x, max.y}, BorderHi, 1.f);
  dl->AddLine({min.x + 1, max.y - 1}, {max.x - 1, max.y - 1}, BorderShadow,
              1.f);
  dl->AddLine({max.x, my}, {max.x, max.y}, BorderHi, 1.f);
  dl->AddLine({max.x - 1, my + 1}, {max.x - 1, max.y - 1}, BorderShadow, 1.f);

  // Label text
  dl->AddText({lx0 + 2, min.y}, TextNormal, label);
}

} // namespace UI::DialogTheme
