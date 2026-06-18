
#pragma once
#include <SDL3/SDL.h>
#include <cstdint>

#include "imgui_impl_sdl3.h"

#include "Document/Canvas.h"
#include "Document/PreviewLayer.h"

#include "Editor/Commands/CommandManager.h"

#include "Editor/Interaction/ToolInteractionState.h"

#include "Editor/Tools/ToolManager.h"

#include "Input/InputDispatcher.h"

#include "Interaction/ToolContext.h"

#include "Rendering/Renderer.h"
#include "Rendering/Transform2D.h"

#include "Viewport/Viewport.h"

struct ToolSettings {
  int brushSize = 1;
  int brushShape = 0;

  bool filledShape = false;

  int lineWidth = 1;

  bool useBackgroundColor = false;
};
class Editor {
private:
  // bool m_panning = false;
  // vec2 m_lastPanMouse{0.0f, 0.0f};
  // bool m_spaceHeld = false;
  uint32_t m_fgColor = COLORS::BLACK;
  uint32_t m_bgColor = COLORS::WHITE;

  Canvas m_canvas;
  PreviewLayer m_preview;
  Renderer m_renderer;
  CommandManager m_commands;
  ToolManager m_tools;
  ToolInteractionState m_interaction;
  InputDispatcher m_input;
  Viewport m_viewport;
  Transform2D m_docTransform;
  void setupTools();
  void setupInputBindings();
  ToolSettings m_toolSettings;
  ToolContext makeToolContext();

  void handleMouseDown(const SDL_Event &e);
  void handleMouseMove(const SDL_Event &e);
  void handleMouseUp(const SDL_Event &e);
  // vec2 screenToCanvas(vec2 screenPos) const;

public:
  explicit Editor(SDL_Renderer *renderer);

  void handleEvent(const SDL_Event &event);
  void update();
  void render();
  void renderUI();

  void centerCanvas();
  void resizeCanvas(int w, int h, const ResizePolicy &policy);

  Canvas &getCanvas() { return m_canvas; }

  void setActiveTool(ToolType tool);
  ToolType getActiveTool() const { return m_tools.getActiveToolType(); }

  void setFgColor(uint32_t color);
  void setBgColor(uint32_t color);

  uint32_t getFgColor() const { return m_fgColor; }
  uint32_t getBgColor() const { return m_bgColor; }

  void setBrushSize(int size) { m_toolSettings.brushSize = size; }
  int getBrushSize() const { return m_toolSettings.brushSize; }
  void setBrushShape(int shape) { m_toolSettings.brushShape = shape; }
  int getBrushShape() const { return m_toolSettings.brushShape; }
  void setFilledShape(bool filled) { m_toolSettings.filledShape = filled; }
  bool getFilledShape() const { return m_toolSettings.filledShape; }
  void setLineWidth(int width) { m_toolSettings.lineWidth = width; }

  int getLineWidth() const { return m_toolSettings.lineWidth; }
  ToolSettings &getToolSettings() { return m_toolSettings; }
  void setUseBackgroundColor(bool value) {
    m_toolSettings.useBackgroundColor = value;
  }

  bool getUseBackgroundColor() const {
    return m_toolSettings.useBackgroundColor;
  }

  vec2 clampToCanvas(vec2 p);
  bool inCanvas(vec2 mousePos);
};
