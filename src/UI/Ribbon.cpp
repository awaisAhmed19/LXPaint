#include "Ribbon.h"
// #include "UILayout.h"
#include "imgui.h"
#include <array>
#include <iterator>
namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 RibbonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

Ribbon::Ribbon(int w, int h) : m_w(w), m_h(h) {}

void Ribbon::raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                          float thickness) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f);
}

void Ribbon::sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                          float thickness) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}

void Ribbon::layout(const ImGuiViewport *vp) {
  constexpr float kRibbonHeight = 21.0f;

  m_rect = {vp->Pos.x, vp->Pos.y, vp->Size.x, kRibbonHeight};
}
float Ribbon::preferredHeight() const {
  ImGuiStyle &style = ImGui::GetStyle();

  return ImGui::GetFontSize() + style.FramePadding.y * 2.0f +
         style.WindowPadding.y * 2.0f;
}
void Ribbon::render() {
  //----------------------------------------------------------------------
  // Layout constants
  //----------------------------------------------------------------------

  constexpr float kBorderThickness = 1.0f;
  constexpr float kRibbonButtonHeight = 21.0f;

  constexpr ImVec2 kFramePadding{10.0f, 2.0f};
  constexpr ImVec2 kWindowPadding{0.0f, 0.0f};
  constexpr ImVec2 kItemSpacing{2.0f, 0.0f};

  constexpr ImGuiWindowFlags kWindowFlags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  static constexpr std::array<const char *, 6> kMenus = {
      "File", "Edit", "View", "Color", "Image", "Help"};

  //----------------------------------------------------------------------
  // Layout
  //----------------------------------------------------------------------

  ImGuiViewport *vp = ImGui::GetMainViewport();

  layout(vp);

  ImGui::SetNextWindowPos({m_rect.x, m_rect.y}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({m_rect.w, m_rect.h}, ImGuiCond_Always);

  //----------------------------------------------------------------------
  // Style
  //----------------------------------------------------------------------

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, kWindowPadding);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kItemSpacing);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, kFramePadding);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::RibbonBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  //----------------------------------------------------------------------
  // Window
  //----------------------------------------------------------------------

  ImGui::Begin("Ribbon", nullptr, kWindowFlags);

  ImDrawList *dl = ImGui::GetWindowDrawList();

  constexpr ImVec2 buttonSize{0.0f, kRibbonButtonHeight};

  for (size_t i = 0; i < kMenus.size(); ++i) {
    ImGui::Button(kMenus[i], buttonSize);

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();

    if (ImGui::IsItemActive())
      sunkenBorder(dl, min, max, kBorderThickness);
    else if (ImGui::IsItemHovered())
      raisedBorder(dl, min, max, kBorderThickness);

    if (i + 1 < kMenus.size())
      ImGui::SameLine();
  }

  //----------------------------------------------------------------------
  // Window border
  //----------------------------------------------------------------------

  const ImVec2 winMin = ImGui::GetWindowPos();
  const ImVec2 winMax = {winMin.x + ImGui::GetWindowWidth(),
                         winMin.y + ImGui::GetWindowHeight()};

  ImGui::End();

  raisedBorder(dl, winMin, winMax, kBorderThickness);

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
}
}; // namespace UI
