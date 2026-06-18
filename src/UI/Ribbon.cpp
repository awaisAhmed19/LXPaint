#include "Ribbon.h"
#include "imgui.h"
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

void Ribbon::raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  // Classic 3D Bevel effect
  drawlist->AddLine(min, {max.x, min.y}, Theme::WHITE, 2.0f); // Top
  drawlist->AddLine(min, {min.x, max.y}, Theme::WHITE, 2.0f); // Left
  drawlist->AddLine({min.x, max.y}, max, Theme::BLACK, 2.0f); // Bottom
  drawlist->AddLine({max.x, min.y}, max, Theme::BLACK, 2.0f); // Right
}
void Ribbon::sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::BLACK, 2.0f);
  drawlist->AddLine(min, {min.x, max.y}, Theme::BLACK, 2.0f);
  drawlist->AddLine({min.x, max.y}, max, Theme::WHITE, 2.0f);
  drawlist->AddLine({max.x, min.y}, max, Theme::WHITE, 2.0f);
}
// static void RibbonButton(const char* btn_id, ImVec2 size,){}

void Ribbon::render() {
  constexpr ImVec2 ButtonPadding = {10, 2};
  const float thickness = 1.0f;
  const float ribbonHeight = 22.0f;

  // ImGui::GetStyle().FramePadding = ButtonPadding;
  ImGuiViewport *vp = ImGui::GetMainViewport();
  const ImVec2 ribbonmin = {vp->Pos.x, vp->Pos.y};
  const ImVec2 ribbonmax = {vp->Size.x, ribbonHeight};
  ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y});
  ImGui::SetNextWindowSize({vp->Size.x, ribbonHeight});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2.f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::RibbonBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  ImGui::Begin("Ribbon", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

  static constexpr const char *Menus[] = {"File",  "Edit",  "View",
                                          "Color", "Image", "Help"};
  ImVec2 buttonSize = {0.0f, 21.0f};

  ImDrawList *drawlist = ImGui::GetWindowDrawList();

  for (size_t i = 0; i < std::size(Menus); ++i) {
    ImGui::Button(Menus[i], buttonSize);

    ImVec2 btnMin = ImGui::GetItemRectMin();
    ImVec2 btnMax = ImGui::GetItemRectMax();
    if (ImGui::IsItemHovered()) {
      raisedBorder(drawlist, btnMin, btnMax);
    }

    if (ImGui::IsItemActive()) {
      sunkenBorder(drawlist, btnMin, btnMax);
    }

    if (i + 1 < std::size(Menus))
      ImGui::SameLine();
  }

  ImGui::End();

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
  raisedBorder(drawlist, ribbonmin, ribbonmax);
}

}; // namespace UI
