#pragma once

#include "imgui.h"
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
//  Footer
//
//  Classic Win95 status bar rendered at the bottom of the window.
//
//  Hover-message flow
//  ──────────────────
//  Any UI widget sets HoverStatus::push(key) while ImGui::IsItemHovered().
//  Footer::render() reads HoverStatus::current() and displays the resolved
//  string.  If nothing is hovered the default text is shown.
//  HoverStatus::endFrame() is called at the end of render() to reset state.
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

class Footer {
public:
  Footer(int w, int h);

  float preferredHeight() const;

  // Live canvas size shown in the right-hand cell.
  // Call this whenever the canvas is resized.
  void setCanvasSize(int w, int h) {
    m_canvasW = w;
    m_canvasH = h;
  }

  // Mouse coordinates shown in the centre cell.
  // Call this from Application::render() with the current canvas-space pos.
  void setMousePos(int x, int y) {
    m_mouseX = x;
    m_mouseY = y;
  }

  void render();

private:
  int m_w = 0;
  int m_h = 0;

  int m_canvasW = 0;
  int m_canvasH = 0;
  int m_mouseX = 0;
  int m_mouseY = 0;

  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);
  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);
};

} // namespace UI
