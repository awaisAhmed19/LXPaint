#include "ColorPallete.h"
#include "imgui.h"
#include <SDL3/SDL.h>
#include <algorithm>
namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 PaletteBg = IM_COL32(192, 192, 192, 255);
} // namespace Theme

// 28 classic MS Paint colors, 2 rows x 14 cols
static constexpr ImVec4 k_palette[28] = {
    // Row 1
    {0.0f, 0.0f, 0.0f, 1.0f},  // Black
    {0.5f, 0.5f, 0.5f, 1.0f},  // Dark Gray
    {0.5f, 0.0f, 0.0f, 1.0f},  // Dark Red
    {0.5f, 0.5f, 0.0f, 1.0f},  // Olive
    {0.0f, 0.5f, 0.0f, 1.0f},  // Dark Green
    {0.0f, 0.5f, 0.5f, 1.0f},  // Teal
    {0.0f, 0.0f, 0.5f, 1.0f},  // Dark Blue
    {0.5f, 0.0f, 0.5f, 1.0f},  // Purple
    {0.5f, 0.25f, 0.0f, 1.0f}, // Brown
    {0.0f, 0.5f, 0.25f, 1.0f}, // Dark Cyan-Green
    {0.0f, 0.75f, 1.0f, 1.0f}, // Sky Blue
    {0.0f, 0.25f, 0.5f, 1.0f}, // Steel Blue
    {0.5f, 0.75f, 1.0f, 1.0f}, // Light Blue
    {0.6f, 0.4f, 0.2f, 1.0f},  // Tan
    // Row 2
    {1.0f, 1.0f, 1.0f, 1.0f},    // White
    {0.75f, 0.75f, 0.75f, 1.0f}, // Light Gray
    {1.0f, 0.0f, 0.0f, 1.0f},    // Red
    {1.0f, 1.0f, 0.0f, 1.0f},    // Yellow
    {0.0f, 1.0f, 0.0f, 1.0f},    // Green
    {0.0f, 1.0f, 1.0f, 1.0f},    // Cyan
    {0.0f, 0.0f, 1.0f, 1.0f},    // Blue
    {1.0f, 0.0f, 1.0f, 1.0f},    // Magenta
    {1.0f, 0.5f, 0.0f, 1.0f},    // Orange
    {0.5f, 1.0f, 0.5f, 1.0f},    // Light Green
    {0.5f, 1.0f, 1.0f, 1.0f},    // Light Cyan
    {0.5f, 0.5f, 1.0f, 1.0f},    // Light Purple
    {1.0f, 0.5f, 0.5f, 1.0f},    // Pink
    {1.0f, 1.0f, 0.5f, 1.0f},    // Light Yellow
};

ColorPallete::ColorPallete(int w, int h) : m_w(w), m_h(h) {}

void ColorPallete::raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                                float thickness) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f);
}

void ColorPallete::sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                                float thickness) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}

uint32_t ColorPallete::toU32(const ImVec4 &color) {
  uint8_t r = static_cast<uint8_t>(color.x * 255.0f);
  uint8_t g = static_cast<uint8_t>(color.y * 255.0f);
  uint8_t b = static_cast<uint8_t>(color.z * 255.0f);
  uint8_t a = static_cast<uint8_t>(color.w * 255.0f);

  return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) |
         (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}
void ColorPallete::drawFgBgSelector(ImDrawList *drawlist, ImVec2 origin) {
  const float bigSize = 22.0f;
  const float smallSize = 16.0f;
  const float offset = 9.0f;

  // Background square (drawn first, behind)
  ImVec2 bgMin = {origin.x + offset, origin.y + offset};
  ImVec2 bgMax = {bgMin.x + bigSize, bgMin.y + bigSize};
  drawlist->AddRectFilled(bgMin, bgMax,
                          ImGui::ColorConvertFloat4ToU32(m_bgColor));
  sunkenBorder(drawlist, bgMin, bgMax);

  // Foreground square (drawn on top)
  ImVec2 fgMin = {origin.x, origin.y};
  ImVec2 fgMax = {fgMin.x + bigSize, fgMin.y + bigSize};
  drawlist->AddRectFilled(fgMin, fgMax,
                          ImGui::ColorConvertFloat4ToU32(m_fgColor));
  sunkenBorder(drawlist, fgMin, fgMax);

  // Click fg square → swap fg/bg (right-click sets bg)
  ImGui::SetCursorScreenPos(fgMin);
  ImGui::InvisibleButton("##fg", {bigSize, bigSize});
  if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    std::swap(m_fgColor, m_bgColor);
  }
}
void ColorPallete::render() {
  const float swatchSize = 16.0f;
  const float swatchGap = 1.5f;
  const int cols = 14;
  const int rows = 2;
  const float selectorW = 52.0f;
  const float paletteH = rows * swatchSize + (rows - 1) * swatchGap;
  const float panelH = paletteH + 35.0f;
  ImGuiViewport *vp = ImGui::GetMainViewport();
  const ImVec2 palleteMin = {vp->Pos.x, vp->Pos.y + 635.0f};
  const ImVec2 palleteMax = {vp->Size.x, vp->Pos.y + 635.f + panelH};
  ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y + 635.0f});
  ImGui::SetNextWindowSize({vp->Size.x, panelH});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {4.0f, 10.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {swatchGap, swatchGap});
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.0f, 0.0f});

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::PaletteBg);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::PaletteBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::PaletteBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::PaletteBg);

  ImGui::Begin("ColorPallete", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  ImVec2 selectorOrigin = {ImGui::GetWindowPos().x + 4.0f,
                           ImGui::GetWindowPos().y + 16.0f};
  drawFgBgSelector(drawlist, selectorOrigin);

  ImGui::SetCursorPos({selectorW + 1.f, 16.0f});
  float swatchStartX = ImGui::GetCursorPosX();

  for (int i = 0; i < cols * rows; ++i) {
    if (i % cols == 0 && i != 0) {
      ImGui::SetCursorPosX(swatchStartX);
    }

    ImGui::PushID(i);

    ImVec2 swatchMin = ImGui::GetCursorScreenPos();
    ImVec2 swatchMax = {swatchMin.x + swatchSize, swatchMin.y + swatchSize};

    drawlist->AddRectFilled(swatchMin, swatchMax,
                            ImGui::ColorConvertFloat4ToU32(k_palette[i]));
    sunkenBorder(drawlist, swatchMin, swatchMax);

    ImGui::InvisibleButton("##swatch", {swatchSize, swatchSize});

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      m_fgColor = k_palette[i];
      m_fgIndex = i;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      m_bgColor = k_palette[i];
      m_bgIndex = i;
    }
    if (i == m_fgIndex) {
      drawlist->AddRect(swatchMin, swatchMax, IM_COL32_WHITE, 0.0f, 0, 2.0f);
    }
    ImGui::PopID();

    if ((i + 1) % cols != 0) {
      ImGui::SameLine();
    }
  }
  ImGui::End();

  ImGui::PopStyleColor(4);
  ImGui::PopStyleVar(4);
  raisedBorder(drawlist, palleteMin, palleteMax, 2.f);
  // SDL_Log("palleteMin.x:%.2f ,palleteMin.y:%.2f", palleteMin.x,
  // palleteMin.y);
  // SDL_Log("palleteMax.x:%.2f ,palleteMax.y:%.2f", palleteMax.x,
  // palleteMax.y);
}

} // namespace UI
