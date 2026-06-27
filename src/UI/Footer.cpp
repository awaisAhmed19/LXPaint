#include "Footer.h"
#include "App/HoverStatus.h"
#include "LayoutEngine/UILayoutConstant.h"
#include "imgui.h"
#include <cstdio>

namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 FooterBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

Footer::Footer(int w, int h) : m_w(w), m_h(h) {}

void Footer::raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f); // top
  dl->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f); // left
  dl->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f); // bottom
  dl->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f); // right
}

void Footer::sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  dl->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  dl->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}

float Footer::preferredHeight() const { return UI::Layout::FooterHeight; }

void Footer::render() {
  constexpr float pad = 1.0f;
  constexpr float gap = 2.0f;
  constexpr float textPad = 3.0f;
  constexpr float textYOff = 2.0f;

  const float footerHeight = preferredHeight();

  ImGuiViewport *vp = ImGui::GetMainViewport();
  const float footerY = vp->Pos.y + vp->Size.y - footerHeight;

  ImGui::SetNextWindowPos({vp->Pos.x, footerY});
  ImGui::SetNextWindowSize({vp->Size.x, footerHeight});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {10.f, 2.f});

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::FooterBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);

  ImGui::Begin("Footer", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus);

  ImDrawList *dl = ImGui::GetWindowDrawList();

  // ── Layout ───────────────────────────────────────────────────────────────

  const ImVec2 origin = ImGui::GetCursorScreenPos();

  const float h = footerHeight - pad * 2.0f;
  const float totalWidth = vp->Size.x - pad * 2.0f;

  const float coordWidth = totalWidth * 0.10f;
  const float sizeWidth = totalWidth * 0.10f;
  // Text cell takes everything that coord and size don't
  const float textWidth = totalWidth - coordWidth - sizeWidth - gap * 2.0f;

  ImVec2 textMin = {origin.x + pad, origin.y + pad};
  ImVec2 textMax = {textMin.x + textWidth, textMin.y + h};

  ImVec2 coordMin = {textMax.x + gap, origin.y + pad};
  ImVec2 coordMax = {coordMin.x + coordWidth, coordMin.y + h};

  ImVec2 sizeMin = {coordMax.x + gap, origin.y + pad};
  ImVec2 sizeMax = {sizeMin.x + sizeWidth, coordMin.y + h};

  // ── Window border ─────────────────────────────────────────────────────────

  ImVec2 footerMin = {vp->Pos.x, footerY};
  ImVec2 footerMax = {vp->Pos.x + vp->Size.x, footerY + footerHeight};
  raisedBorder(dl, footerMin, footerMax);

  // ── Cell borders ──────────────────────────────────────────────────────────

  sunkenBorder(dl, textMin, textMax);
  sunkenBorder(dl, coordMin, coordMax);
  sunkenBorder(dl, sizeMin, sizeMax);

  // ── Help / hover text (left cell) ─────────────────────────────────────────

  // Determine which text to show.
  // HoverStatus::current() returns the message for whatever widget was
  // hovered this frame, or nullptr when nothing is hovered.
  static const std::string kDefault =
      "For Help, click Help Topics on the Help Menu.";
  const std::string *hoverMsg = HoverStatus::current();
  const std::string &helpText = hoverMsg ? *hoverMsg : kDefault;

  dl->PushClipRect(textMin, textMax, true);
  dl->AddText({textMin.x + textPad, textMin.y + textYOff}, Theme::TextColor,
              helpText.c_str());
  dl->PopClipRect();

  // ── Coordinates (centre cell) ─────────────────────────────────────────────

  char coordBuf[32];
  if (m_mouseX >= 0 && m_mouseY >= 0)
    std::snprintf(coordBuf, sizeof(coordBuf), "%d,%d", m_mouseX, m_mouseY);
  else
    coordBuf[0] = '\0';

  dl->PushClipRect(coordMin, coordMax, true);
  dl->AddText({coordMin.x + textPad, coordMin.y + textYOff}, Theme::TextColor,
              coordBuf);
  dl->PopClipRect();

  // ── Canvas size (right cell) ──────────────────────────────────────────────

  char sizeBuf[32];
  if (m_canvasW > 0 && m_canvasH > 0)
    std::snprintf(sizeBuf, sizeof(sizeBuf), "%dx%d", m_canvasW, m_canvasH);
  else
    sizeBuf[0] = '\0';

  dl->PushClipRect(sizeMin, sizeMax, true);
  dl->AddText({sizeMin.x + textPad, sizeMin.y + textYOff}, Theme::TextColor,
              sizeBuf);
  dl->PopClipRect();

  ImGui::End();

  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(4);

  // ── Reset hover state for next frame ──────────────────────────────────────
  // This must happen AFTER drawing so nothing written this frame leaks
  // into the next frame if no widget pushes a message.
  HoverStatus::endFrame();
}

} // namespace UI
