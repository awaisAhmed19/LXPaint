#pragma once
#include <SDL3/SDL.h>
#include <cstdint>

#include "App/Globals.h"
#include "Editor/Tools/ClickTool.h"
#include "UI/LayoutEngine/LayoutMetrics.h"
#include "imgui_impl_sdl3.h"

#include "Editor/Commands/CommandManager.h"
#include "EditorDocument.h"

#include "Editor/Interaction/ToolInteractionState.h"

#include "Editor/Tools/ToolManager.h"

#include "Input/InputDispatcher.h"

#include "Interaction/ToolContext.h"
#include "ToolSettings.h"

#include "Rendering/Renderer.h"
#include "Rendering/Transform2D.h"

#include "Viewport/Viewport.h"

class Editor {
private:
  // bool m_panning = false;
  // vec2 m_lastPanMouse{0.0f, 0.0f};
  // bool m_spaceHeld = false;
  uint32_t m_fgColor = COLORS::BLACK;
  uint32_t m_bgColor = COLORS::WHITE;

  SDL_Window *m_window = nullptr;
  EditorDocument m_document;
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
  explicit Editor(SDL_Window *window, SDL_Renderer *renderer,
                  const UI::LayoutMetrics &layout);

  void handleEvent(const SDL_Event &event);
  void update();
  void render();
  void renderUI();

  void centerCanvas();
  void resizeCanvas(int w, int h, const ResizePolicy &policy);

  void setActiveTool(ToolType tool);
  ToolType getActiveTool() const { return m_tools.getActiveToolType(); }
  void setViewportRect(SDL_FRect rect);
  void setFgColor(uint32_t color);
  void setBgColor(uint32_t color);

  uint32_t getFgColor() const { return m_fgColor; }
  uint32_t getBgColor() const { return m_bgColor; }

  void setBrushSize(int size) { m_toolSettings.strokeWidth = size; }
  int getBrushSize() const { return m_toolSettings.strokeWidth; }
  void setBrushShape(ToolSettings::BrushShape shape) {
    m_toolSettings.brushShape = shape;
  }
  ToolSettings::BrushShape getBrushShape() const {
    return m_toolSettings.brushShape;
  }
  // void setFilledShape(bool filled) { m_toolSettings.fillShapes = filled; }
  // bool getFilledShape() const { return m_toolSettings.fillShapes; }
  void setLineWidth(int width) { m_toolSettings.lineWidth = width; }

  int getLineWidth() const { return m_toolSettings.lineWidth; }
  ToolSettings &getToolSettings() { return m_toolSettings; }
  /*
   void setUseBackgroundColor(bool value) {
     m_toolSettings.useBackgroundColor = value;
   }

   bool getUseBackgroundColor() const {
     return m_toolSettings.useBackgroundColor;
   }
 */

  // ── Undo / Redo ─────────────────────────────────────────────────────
  // Thin pass-through to CommandManager, operating on the active
  // document's canvas. This is the one place that knows how to turn
  // "undo the last thing" into the concrete (CommandManager, Canvas)
  // pair — Ctrl+Z/Y (setupInputBindings) and the Edit menu
  // (MenuActionDispatcher) both call through here instead of each
  // reaching into m_commands / m_document themselves.
  bool undo() { return m_commands.undo(m_document.getCanvas()); }
  bool redo() { return m_commands.redo(m_document.getCanvas()); }
  bool canUndo() const { return m_commands.canUndo(); }
  bool canRedo() const { return m_commands.canRedo(); }

  vec2 clampToCanvas(vec2 p);
  bool inCanvas(vec2 mousePos);
};
