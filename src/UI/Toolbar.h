#pragma once
#include "App/Globals.h"
#include "Editor/Editor.h"
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

  void raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max);
  void sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max);

  // options panel renderers
  void renderOptions(Editor &editor, ImDrawList *dl);
  void renderSizeDots(Editor &editor, ImDrawList *dl, ImVec2 origin,
                      const int *sizes, int count);
  void renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin);
  void renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin);

public:
  Toolbar(int w, int h);
  ~Toolbar();

  // toolMin / toolMax now drive the full window including options panel
  ImVec2 toolMin = {0.0f, 23.0f};
  ImVec2 toolMax = {66.0f, 635.0f};
  bool init(SDL_Renderer *renderer);
  void render(Editor &editor); // now takes editor ref

  ToolType getActiveTool() const { return m_activeTool; }
};

} // namespace UI
