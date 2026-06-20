#include "Footer.h"
#include "LayoutEngine/UILayoutConstant.h"
#include "imgui.h"
namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 FooterBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

Footer::Footer(int w, int h) : m_w(w), m_h(h) {}

void Footer::raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f); // Top
  drawlist->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f); // Left
  drawlist->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f); // Bottom
  drawlist->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f); // Right
}
void Footer::sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}
/*
float Footer::preferredHeight() const {
  ImGuiStyle &style = ImGui::GetStyle();

  return ImGui::GetFontSize() + style.FramePadding.y * 2.0f +
         style.WindowPadding.y * 2.0f;
}
*/
float Footer::preferredHeight() const { return UI::Layout::FooterHeight; }
void Footer::render() {
  constexpr ImVec2 ButtonPadding = {10, 2};
  constexpr float pad = 1.0f;
  constexpr float gap = 2.0f;

  const float footerHeight = preferredHeight();

  ImGuiViewport *vp = ImGui::GetMainViewport();
  const float footerY = vp->Pos.y + vp->Size.y - footerHeight;

  ImGui::SetNextWindowPos({vp->Pos.x, footerY});
  ImGui::SetNextWindowSize({vp->Size.x, footerHeight});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::FooterBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  ImGui::Begin("Footer", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus);

  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  const ImVec2 origin = ImGui::GetCursorScreenPos();

  const float h = footerHeight - pad * 2.0f;
  const float totalWidth = vp->Size.x - pad * 2.0f;

  const float textWidth = totalWidth * 0.80f;
  const float coordWidth = totalWidth * 0.10f;
  const float sizeWidth = totalWidth - textWidth - coordWidth - gap * 2.0f;

  ImVec2 textMin = {origin.x + pad, origin.y + pad};
  ImVec2 textMax = {textMin.x + textWidth, textMin.y + h};

  ImVec2 coordMin = {textMax.x + gap, origin.y + pad};
  ImVec2 coordMax = {coordMin.x + coordWidth, coordMin.y + h};

  ImVec2 sizeMin = {coordMax.x + gap, origin.y + pad};
  ImVec2 sizeMax = {sizeMin.x + sizeWidth, coordMin.y + h};

  //----------------------------------------------------
  // Window border
  //----------------------------------------------------

  ImVec2 footerMin = {vp->Pos.x, footerY};
  ImVec2 footerMax = {vp->Pos.x + vp->Size.x, footerY + footerHeight};

  raisedBorder(drawlist, footerMin, footerMax);

  //----------------------------------------------------
  // Cell borders
  //----------------------------------------------------

  sunkenBorder(drawlist, textMin, textMax);
  sunkenBorder(drawlist, coordMin, coordMax);
  sunkenBorder(drawlist, sizeMin, sizeMax);

  //----------------------------------------------------
  // Clipped text
  //----------------------------------------------------

  constexpr float textPad = 3.0f;
  constexpr float textYOffset = 2.0f;

  // Help text
  drawlist->PushClipRect(textMin, textMax, true);
  drawlist->AddText({textMin.x + textPad, textMin.y + textYOffset},
                    Theme::BLACK, "Click on Help to know more.");
  drawlist->PopClipRect();

  // Coordinates
  drawlist->PushClipRect(coordMin, coordMax, true);
  drawlist->AddText({coordMin.x + textPad, coordMin.y + textYOffset},
                    Theme::BLACK, "X:100  Y:100");
  drawlist->PopClipRect();

  // Canvas size
  drawlist->PushClipRect(sizeMin, sizeMax, true);
  drawlist->AddText({sizeMin.x + textPad, sizeMin.y + textYOffset},
                    Theme::BLACK, "100 x 100");
  drawlist->PopClipRect();
  ImGui::End();

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
}
}; // namespace UI
