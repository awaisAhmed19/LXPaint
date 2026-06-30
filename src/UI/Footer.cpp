#include "Footer.h"
#include "BorderRenderer.h"
#include "HoverStatus.h"
#include "LayoutEngine/UILayoutConstant.h"
#include "RetroWindow.h"
#include "Theme.h"
#include "imgui.h"
#include <cstdio>

namespace UI {

Footer::Footer(int w, int h) : m_w(w), m_h(h) {}

float Footer::preferredHeight() const { return UI::Layout::FooterHeight; }

void Footer::render() {
  constexpr float pad = 1.0f;
  constexpr float gap = 2.0f;
  constexpr float textPad = 3.0f;
  constexpr float textYOff = 2.0f;

  const float footerHeight = preferredHeight();

  ImGuiViewport *vp = ImGui::GetMainViewport();
  const float footerY = vp->Pos.y + vp->Size.y - footerHeight;

  // ── Window ───────────────────────────────────────────────────────────
  // Replaces the manual PushStyleVar×4 / PushStyleColor×2 / Begin / End /
  // Pop dance with the shared RAII wrapper — same visual result, no
  // duplicated boilerplate.
  RetroWindowDesc desc;
  desc.pos = {vp->Pos.x, footerY};
  desc.size = {vp->Size.x, footerHeight};
  desc.itemSpacing = {gap, 0.f};
  desc.framePadding = {10.f, 2.f};
  desc.bg = Theme::WindowBg;
  desc.text = Theme::TextColor;

  RetroWindow win("Footer", desc);
  ImDrawList *dl = win.drawList();

  // ── Layout ───────────────────────────────────────────────────────────

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

  // ── Window border ───────────────────────────────────────────────────

  BorderRenderer::Raised(dl, win.min(), win.max());

  // ── Cell borders ────────────────────────────────────────────────────

  BorderRenderer::Sunken(dl, textMin, textMax);
  BorderRenderer::Sunken(dl, coordMin, coordMax);
  BorderRenderer::Sunken(dl, sizeMin, sizeMax);

  // ── Help / hover text (left cell) ───────────────────────────────────

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

  // ── Coordinates (centre cell) ───────────────────────────────────────
  // m_mouseX/m_mouseY are canvas-space coordinates supplied by the Editor
  // via setMousePos() — Footer never computes screen→canvas math itself,
  // it only displays whatever it's told. Negative values (set when the
  // mouse is outside the canvas) render as a blank cell rather than a
  // misleading "0,0" or stale coordinate.

  char coordBuf[32];
  if (m_mouseX >= 0 && m_mouseY >= 0)
    std::snprintf(coordBuf, sizeof(coordBuf), "%d,%d", m_mouseX, m_mouseY);
  else
    coordBuf[0] = '\0';

  dl->PushClipRect(coordMin, coordMax, true);
  dl->AddText({coordMin.x + textPad, coordMin.y + textYOff}, Theme::TextColor,
              coordBuf);
  dl->PopClipRect();

  // ── Canvas size (right cell) ────────────────────────────────────────

  char sizeBuf[32];
  if (m_canvasW > 0 && m_canvasH > 0)
    std::snprintf(sizeBuf, sizeof(sizeBuf), "%dx%d", m_canvasW, m_canvasH);
  else
    sizeBuf[0] = '\0';

  dl->PushClipRect(sizeMin, sizeMax, true);
  dl->AddText({sizeMin.x + textPad, sizeMin.y + textYOff}, Theme::TextColor,
              sizeBuf);
  dl->PopClipRect();

  // RetroWindow's destructor (end of scope, right after this block) calls
  // ImGui::End() and pops every style var/color pushed in its constructor.

  // ── Reset hover state for next frame ────────────────────────────────
  // This must happen AFTER drawing so nothing written this frame leaks
  // into the next frame if no widget pushes a message.
  HoverStatus::endFrame();
}

} // namespace UI
