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

  void setFgColor(const ImVec4 &color) { m_fgColor = color; }

  // ImU32 / ImGui color (IM_COL32 or GetColorU32)
  void setFgColor(ImU32 color) {
    m_fgColor = ImGui::ColorConvertU32ToFloat4(color);
  }

  // RGBA bytes
  void setFgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    m_fgColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }

  enum class HexFormat { RRGGBBAA, AARRGGBB };

  void setFgColor(uint32_t hex, HexFormat fmt = HexFormat::AARRGGBB) {
    if (fmt == HexFormat::AARRGGBB) {
      setFgColor((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF,
                 (hex >> 24) & 0xFF);
    } else {
      setFgColor((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF,
                 hex & 0xFF);
    }
  }

  void setBgColor(const ImVec4 &color) { m_bgColor = color; }

  void setBgColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    m_bgColor = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }

  void setBgColor(uint32_t hex, HexFormat fmt = HexFormat::AARRGGBB) {
    if (fmt == HexFormat::AARRGGBB) {
      setBgColor((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF,
                 (hex >> 24) & 0xFF);
    } else {
      setBgColor((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF,
                 hex & 0xFF);
    }
  }
};

} // namespace UI
