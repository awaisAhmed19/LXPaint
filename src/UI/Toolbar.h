#pragma once
#include "App/Globals.h"
#include "imgui.h"
#include <SDL3/SDL.h>
namespace UI {

class Toolbar {
private:
  int m_w = 0;
  int m_h = 0;

  static constexpr int TotalButtons = 16;
  SDL_Texture *m_textures[TotalButtons] = {nullptr};
  ToolType m_activeTool = ToolType::Pencil;
  void raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);
  void sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max);

public:
  Toolbar(int w, int h);
  ~Toolbar();
  ImVec2 toolMin = {0, 23.0};
  ImVec2 toolMax = {66.0, 635.0};
  bool init(SDL_Renderer *renderer);
  void render();

  ToolType getActiveTool() const { return m_activeTool; }
};

} // namespace UI
