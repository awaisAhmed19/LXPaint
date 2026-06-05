#include "Theme.h"
#include "imgui.h"
#include <algorithm>
#include <string>
void LXTheme::applyMainTheme() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_WindowBg] = LXTheme::Bgcolor;

  colors[ImGuiCol_MenuBarBg] = LXTheme::MainColor;

  colors[ImGuiCol_Button] = LXTheme::MainColor;
  colors[ImGuiCol_ButtonHovered] = LXTheme::White;
  colors[ImGuiCol_ButtonActive] = LXTheme::BoardDark;

  colors[ImGuiCol_Border] = LXTheme::Black;
  style.FrameBorderSize = 1.0f;
}

// Theme.cpp
bool LXButton::Draw(const char *label, ImVec2 size, ImDrawList *drawList) {
  std::string id = std::string(label) + "##retro";

  bool pressed = ImGui::InvisibleButton(id.c_str(), {size.x - 1, size.y - 1});
  bool active = ImGui::IsItemActive();
  bool hovered = ImGui::IsItemHovered();

  // Use provided draw list (foreground) or fall back to window draw list
  ImDrawList *draw = drawList ? drawList : ImGui::GetWindowDrawList();

  ImVec2 min = ImGui::GetItemRectMin();
  ImVec2 max = ImGui::GetItemRectMax();
  float actualW = max.x - min.x;
  float actualH = max.y - min.y;

  ImVec2 textSize = ImGui::CalcTextSize(label);

  ImU32 light = ImGui::ColorConvertFloat4ToU32(LXTheme::White);
  ImU32 dark = ImGui::ColorConvertFloat4ToU32(LXTheme::Black);
  ImU32 fill = ImGui::ColorConvertFloat4ToU32(LXTheme::MainColor);

  if (active)
    std::swap(light, dark);

  // draw->AddRect(min, max);

  if (hovered) {
    draw->AddLine(min, {max.x, min.y}, light);
    draw->AddLine(min, {min.x, max.y}, light);
    draw->AddLine({min.x, max.y - 1}, {max.x, max.y - 1}, dark);
    draw->AddLine({max.x - 1, min.y}, {max.x - 1, max.y}, dark);
  }

  ImVec2 textPos = {min.x + (actualW - textSize.x) * 0.5f,
                    min.y + (actualH - textSize.y) * 0.5f};
  if (active) {
    textPos.x += 1.0f;
    textPos.y += 1.0f;
  }

  draw->AddText(textPos, IM_COL32_BLACK, label);
  return pressed;
}
