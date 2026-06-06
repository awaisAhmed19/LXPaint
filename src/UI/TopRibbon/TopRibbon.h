#pragma once
#include <functional>
#include <imgui.h>
#include <string>
#include <vector>

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
  std::function<void()> onActivate;
};

class TopRibbon {
public:
  TopRibbon();
  void render();

  void bindMenu(MenuType type, std::function<void()> fn);

private:
  std::vector<MenuEntry> m_menus;
  void drawBackground(ImDrawList *draw, ImVec2 rbMin, ImVec2 rbMax) const;
};
