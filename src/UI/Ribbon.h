#include "imgui.h"
#include <vector>
namespace UI {

struct MenuItems {
  const char *label;
  const char *shortcut;
  bool isHovered = false;
  bool isPressed = false;
};

// std::vector<MenuItems> menus = {{"New", "Ctrl+N", false, false}};

class Ribbon {
private:
  const int m_x = 0;
  const int m_y = 0;
  int m_w = 0;
  int m_h = 0;
  ImU32 m_col = IM_COL32(192, 192, 192, 255);

  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);

  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);

public:
  Ribbon(int w, int h);
  void render();
};
}; // namespace UI
