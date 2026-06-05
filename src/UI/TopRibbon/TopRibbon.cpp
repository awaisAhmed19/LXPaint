#include "TopRibbon.h"
#include "UI/Theme.h"
#include <imgui.h>

namespace {

constexpr MenuEntry Menus[] = {
    {"File", MenuType::File},   {"Edit", MenuType::Edit},
    {"View", MenuType::View},   {"Image", MenuType::Image},
    {"Color", MenuType::Color}, {"Help", MenuType::Help},
};

constexpr float RibbonHeight = 20.0f;
constexpr float ButtonWidth = 45.0f;

} // namespace

void TopRibbon::drawBorder(ImDrawList *draw, const ImVec2 &min,
                           const ImVec2 &max) {
  ImU32 light = ImGui::ColorConvertFloat4ToU32(LXTheme::White);
  ImU32 dark = ImGui::ColorConvertFloat4ToU32(LXTheme::Black);
  draw->AddLine({min.x, min.y}, {max.x, min.y}, light);
  draw->AddLine({min.x, min.y}, {min.x, max.y}, light);
  draw->AddLine({min.x, max.y - 1}, {max.x, max.y - 1}, dark);
  draw->AddLine({max.x - 1, min.y}, {max.x - 1, max.y}, dark);
}

void TopRibbon::render() {
  ImGuiViewport *vp = ImGui::GetMainViewport();

  const float fontHeight = ImGui::GetTextLineHeight();
  const float btnPadY = 4.0f;
  const float ribbonHeight = fontHeight + btnPadY * 2.5f;

  ImVec2 origin = vp->Pos; // top-left of screen
  ImVec2 rbMin = origin;
  ImVec2 rbMax = {origin.x + vp->Size.x, origin.y + ribbonHeight};

  ImDrawList *draw = ImGui::GetForegroundDrawList();
  draw->AddRectFilled(rbMin, rbMax,
                      ImGui::ColorConvertFloat4ToU32(LXTheme::MainColor));
  drawBorder(draw, rbMin, rbMax);

  // Buttons — use InvisibleButton on a dummy window that exactly fits the
  // ribbon so hit-testing works, but WE control all drawing via the foreground
  // list
  ImGui::SetNextWindowPos(rbMin);
  ImGui::SetNextWindowSize({vp->Size.x, ribbonHeight});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_WindowBg,
                        ImVec4(0, 0, 0, 0)); // fully transparent

  ImGui::Begin("##TopRibbonHitArea", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBackground |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  const ImVec2 buttonSize = {ButtonWidth, ribbonHeight};
  float x = 0.0f;

  for (const auto &menu : Menus) {
    ImGui::SetCursorPos({x, 0.0f});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    // Pass the foreground draw list so LXButton draws on top of everything
    if (LXButton::Draw(menu.label, buttonSize, draw)) { /* dispatch */
    }
    ImGui::PopStyleVar();
    x += ButtonWidth;
  }

  ImGui::End();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(2);
}
