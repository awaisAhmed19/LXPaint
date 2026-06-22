#pragma once
#include <cstdint>
class Canvas;
#pragma once
#include <cstdint>
class Canvas;
class PreviewLayer;

struct ToolInteractionState;
struct ToolSettings;
class CommandManager;
class Viewport;

struct ToolContext {
  Canvas *canvas;
  PreviewLayer *preview;
  ToolInteractionState *interaction;

  CommandManager *commandManager;

  uint32_t fgColor;
  uint32_t bgColor;
  int brushSize = 1;
  ToolSettings *settings;

  // Write-back pointers into Editor's live color state.
  // Tools that change the active fg/bg color (Eyedropper) write through
  // these instead of mutating the ctx copies above, which are snapshots
  // taken at dispatch time via Editor::makeToolContext().
  uint32_t *fgColorOut = nullptr;
  uint32_t *bgColorOut = nullptr;

  // Needed by Magnifier (zoom in/out at click point) and Text (knowing the
  // doc transform / screen mapping for placing the editable rect is handled
  // by Editor before calling the tool, so only Viewport is required here).
  Viewport *viewport = nullptr;
};
