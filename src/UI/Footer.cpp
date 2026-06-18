#include "Footer.h"
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

float Footer::preferredHeight() const {
  ImGuiStyle &style = ImGui::GetStyle();

  return ImGui::GetFontSize() + style.FramePadding.y * 2.0f +
         style.WindowPadding.y * 2.0f;
}

void Footer::render() {
  constexpr ImVec2 ButtonPadding = {10, 2};
  const float thickness = 1.0f;
  const float footerHeight = 22.0f;

  // ImGui::GetStyle().FramePadding = ButtonPadding;
  ImGuiViewport *vp = ImGui::GetMainViewport();
  const ImVec2 footermax = {vp->Size.x, vp->Size.y};
  ImGui::SetNextWindowPos({footermin.x, footermin.y});
  ImGui::SetNextWindowSize({vp->Size.x, footerHeight});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2.f, 0.0f});
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

  const float pad = 1.0f;
  const float gap = 2.0f;

  const float h = footerHeight - 2.0f * pad;

  const float totalWidth = vp->Size.x - 2.0f * pad;

  const float textWidth = totalWidth * 0.80f;
  const float coordWidth = totalWidth * 0.10f;
  const float sizeWidth = totalWidth - textWidth - coordWidth - 2.0f * gap;

  ImVec2 textMin = {origin.x + pad, origin.y + pad};
  ImVec2 textMax = {textMin.x + textWidth, textMin.y + h};

  ImVec2 coordMin = {textMax.x + gap, origin.y + pad};
  ImVec2 coordMax = {coordMin.x + coordWidth, coordMin.y + h};

  ImVec2 sizeMin = {coordMax.x + gap, origin.y + pad};
  ImVec2 sizeMax = {sizeMin.x + sizeWidth, sizeMin.y + h};
  drawlist->AddText({textMin.x + 3, textMin.y + 2}, Theme::BLACK,
                    "Click on Help to know more.");

  drawlist->AddText({coordMin.x + 3, coordMin.y + 2}, Theme::BLACK,
                    "X:100  Y:100");

  drawlist->AddText({sizeMin.x + 3, sizeMin.y + 2}, Theme::BLACK, "100 x 100");
  ImGui::End();

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
  raisedBorder(drawlist, footermin, footermax);
  sunkenBorder(drawlist, textMin, textMax);
  sunkenBorder(drawlist, coordMin, coordMax);
  sunkenBorder(drawlist, sizeMin, sizeMax);
}

}; // namespace UI
