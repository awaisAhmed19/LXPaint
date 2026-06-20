#include "imgui.h"
namespace UI {
class Footer {
private:
  const int m_x = 0;
  const int m_y = 0;
  int m_w = 0;
  int m_h = 0;
  ImU32 m_col = IM_COL32(192, 192, 192, 255);

  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);

  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);

public:
  Footer(int w, int h);
  float preferredHeight() const;
  void render();
};
}; // namespace UI
