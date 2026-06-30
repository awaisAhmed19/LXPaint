#pragma once
#include "App/Globals.h"
#include "Editor/Editor.h"
#include "UI/LayoutEngine/LayoutMetrics.h"
#include "imgui.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <cstddef>

namespace UI {

class Toolbar {
private:
  int m_w = 0;
  int m_h = 0;
  static constexpr int TotalButtons = 16;
  SDL_Texture *m_textures[TotalButtons] = {nullptr};
  SDL_Texture *m_backgroundTransparentIcon = nullptr;
  SDL_Texture *m_backgroundOpaqueIcon = nullptr;
  SDL_Renderer *m_renderer = nullptr;
  ToolType m_activeTool = ToolType::Pencil;

  void renderOptions(Editor &editor, ImDrawList *dl, const PanelRect &box);
  void renderSizeSquares(Editor &editor, ImDrawList *dl, ImVec2 origin,
                         float optionWidth, float optionHeight,
                         const int *sizes, int count);
  void renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                         float optionWidth, float optionHeight);
  void renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                       float optionWidth, float optionHeight);
  void renderLineWidths(Editor &editor, ImDrawList *dl, ImVec2 origin,
                        float optionWidth, float optionHeight);
  void renderAirbrushSizes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                           float optionWidth, float optionHeight);
  void renderZoomLevels(Editor &editor, ImDrawList *dl, ImVec2 origin,
                        float optionWidth, float optionHeight);
  void renderBackgroundModeIcons(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                 float optionWidth, float optionHeight,
                                 ToolSettings::BackgroundMode &target);

public:
  Toolbar(int w, int h, SDL_Renderer *renderer);
  ~Toolbar();
  bool init();

  // Geometry now comes entirely from LayoutMetrics::toolbarMetrics —
  // recomputed every frame by LayoutEngine, so resizing the window updates
  // the toolbar on the very next frame with no extra plumbing.
  void render(Editor &editor, const LayoutMetrics &layout);

  ToolType getActiveTool() const { return m_activeTool; }
};

} // namespace UI
