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

  void raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max,
                    float thickness = 1.f);
  void sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max,
                    float thickness = 1.f);

  void renderOptions(Editor &editor, ImDrawList *dl);
  void renderSizeSquares(Editor &editor, ImDrawList *dl, ImVec2 origin,
                         float optionWidth, float optionHeight,
                         const int *sizes, int count);
  void renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin);
  void renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                       float optionWidth, float optionHeight);
  void renderLineWidths(Editor &editor, ImDrawList *dl, ImVec2 origin,
                        float optionWidth, float optionHeight);
  void renderAirbrushSizes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                           float optionWidth, float optionHeight);
  void renderZoomLevels(Editor &editor, ImDrawList *dl, ImVec2 origin);
  void renderBackgroundModeIcons(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                 ToolSettings::BackgroundMode &target);

public:
  Toolbar(int w, int h);
  ~Toolbar();
  float preferredWidth() const;
  ImVec2 toolMin = {0, 23};
  ImVec2 toolMax = {66, 615};
  bool init(SDL_Renderer *renderer);
  void render(Editor &editor);

  ToolType getActiveTool() const { return m_activeTool; }
};

} // namespace UI
