#pragma once
#include <imgui.h>
#include <span>

enum class MenuType {
  File,
  Edit,
  View,
  Image,
  Color,
  Help,
};

struct MenuEntry {
  const char *label;
  MenuType type;
};

class TopRibbon {
public:
  void render();

private:
  void drawBorder(ImDrawList *draw, const ImVec2 &min, const ImVec2 &max);
};
