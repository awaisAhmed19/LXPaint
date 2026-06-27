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

// Forward declare to avoid including the full dialog headers in Editor.h
namespace UI {
class DialogManager;
}

class Editor {
private:
  bool m_fullscreen = false;
  bool m_toolboxVisible = true;
  bool m_paletteVisible = true;
  bool m_statusBarVisible = true;

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
  ToolSettings m_toolSettings;

  // Dialog manager pointer — set by Application after construction.
  // Never null after setDialogManager() has been called; every call site
  // that opens a dialog must check this is non-null before calling through.
  UI::DialogManager *m_dialogManager = nullptr;

  void setupTools();
  void setupInputBindings();
  ToolContext makeToolContext();

  void handleMouseDown(const SDL_Event &e);
  void handleMouseMove(const SDL_Event &e);
  void handleMouseUp(const SDL_Event &e);

public:
  explicit Editor(SDL_Window *window, SDL_Renderer *renderer,
                  const UI::LayoutMetrics &layout);

  // ── Dialog manager wiring ─────────────────────────────────────────────────
  void setDialogManager(UI::DialogManager *mgr) { m_dialogManager = mgr; }
  UI::DialogManager *dialogManager() const { return m_dialogManager; }

  // ── Core ──────────────────────────────────────────────────────────────────
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
  void setLineWidth(int width) { m_toolSettings.lineWidth = width; }
  int getLineWidth() const { return m_toolSettings.lineWidth; }
  ToolSettings &getToolSettings() { return m_toolSettings; }

  // ── Canvas size (used by dialogs, e.g. ImageAttributes) ──────────────────
  int getCanvasWidth() const { return m_document.getCanvas().getWidth(); }
  int getCanvasHeight() const { return m_document.getCanvas().getHeight(); }

  // ── Document ──────────────────────────────────────────────────────────────
  void newDocument();
  bool saveDocument();
  bool saveDocumentAs();
  bool openDocument();

  // ── Undo / Redo ───────────────────────────────────────────────────────────
  bool undo() { return m_commands.undo(m_document.getCanvas()); }
  bool redo() { return m_commands.redo(m_document.getCanvas()); }
  bool canUndo() const { return m_commands.canUndo(); }
  bool canRedo() const { return m_commands.canRedo(); }

  // ── Image menu ────────────────────────────────────────────────────────────
  void invertColors();
  void flipHorizontal();
  void flipVertical();
  void rotate90CW();
  void rotate90CCW();
  void clearImage();

  // ── Edit menu ─────────────────────────────────────────────────────────────
  void selectAll();
  void clearSelection();

  // ── View menu ─────────────────────────────────────────────────────────────
  void setFullscreen(bool fullscreen);
  bool isFullscreen() const { return m_fullscreen; }

  void setToolboxVisible(bool visible) { m_toolboxVisible = visible; }
  bool isToolboxVisible() const { return m_toolboxVisible; }

  void setPaletteVisible(bool visible) { m_paletteVisible = visible; }
  bool isPaletteVisible() const { return m_paletteVisible; }

  void setStatusBarVisible(bool visible) { m_statusBarVisible = visible; }
  bool isStatusBarVisible() const { return m_statusBarVisible; }

  // ── Helpers ───────────────────────────────────────────────────────────────
  vec2 clampToCanvas(vec2 p);
  bool inCanvas(vec2 mousePos);

  bool isModified() const { return m_document.isModified(); }
};
