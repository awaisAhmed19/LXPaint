#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>

constexpr ImVec4 rgb(int r, int g, int b, int a = 255) {
  return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
namespace LXTheme {
inline const ImVec4 Bgcolor = rgb(128, 128, 128);
inline const ImVec4 MainColor = rgb(192, 192, 192);
inline const ImVec4 White = rgb(255, 255, 255);
inline const ImVec4 Black = rgb(0, 0, 0);
inline const ImVec4 BoardLight = rgb(128, 128, 128);
inline const ImVec4 BoardDark = rgb(128, 128, 128);
constexpr SDL_Color AppBackground = {128, 128, 128, 255};

void applyMainTheme();
}; // namespace LXTheme
   //
namespace LXButton {
bool Draw(const char *label, ImVec2 size, ImDrawList *draw = nullptr);
}
