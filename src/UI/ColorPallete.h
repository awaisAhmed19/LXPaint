#pragma once
#include "imgui.h"

namespace UI {

class ColorPallete {
private:
  int m_w = 0;
  int m_h = 0;

  ImVec4 m_fgColor = {0.0f, 0.0f, 0.0f, 1.0f}; // Foreground (black)
  ImVec4 m_bgColor = {1.0f, 1.0f, 1.0f, 1.0f}; // Background (white)

  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);
  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);
  void drawFgBgSelector(ImDrawList *drawlist, ImVec2 origin);

public:
  ColorPallete(int w, int h);
  void render();

  ImVec4 getFgColor() const { return m_fgColor; }
  ImVec4 getBgColor() const { return m_bgColor; }
};

} // namespace UI
