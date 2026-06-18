#pragma once
#include "imgui.h"
#include <stdint.h>
namespace UI {

class ColorPalette {
private:
  int m_w = 0;
  int m_h = 0;
  int m_fgIndex = 0;                           // Black
  int m_bgIndex = 14;                          // White
  ImVec4 m_fgColor = {0.0f, 0.0f, 0.0f, 1.0f}; // Foreground (black)
  ImVec4 m_bgColor = {1.0f, 1.0f, 1.0f, 1.0f}; // Background (white)

  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                    float thickness = 1.0f);
  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max,
                    float thickness = 1.0f);
  void drawFgBgSelector(ImDrawList *drawlist, ImVec2 origin);

public:
  ColorPalette(int w, int h);
  void render();
  float preferredHeight() const;
  static uint32_t toU32(const ImVec4 &color);
  ImVec4 getFgColor() const { return m_fgColor; }
  ImVec4 getBgColor() const { return m_bgColor; }
};

} // namespace UI
