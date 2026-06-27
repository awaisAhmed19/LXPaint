#pragma once

#include "App/Globals.h"
#include "Document/Canvas.h"
#include "Document/PreviewLayer.h"
#include "Editor/Commands/CommandManager.h"
#include "Editor/Interaction/ToolInteractionState.h"
#include "Editor/ToolSettings.h"

// Forward declarations
class Viewport;
struct SDL_Window;

// ─────────────────────────────────────────────────────────────────────────────
//  ToolContext
//
//  Passed by value into every tool event handler. Contains read-only
//  snapshots of editor state (fgColor, bgColor) plus write-back pointers
//  for the tools that need to mutate editor state directly (Eyedropper,
//  Magnifier). Avoids giving tools a raw Editor reference.
// ─────────────────────────────────────────────────────────────────────────────

struct ToolContext {
  Canvas *canvas = nullptr;
  PreviewLayer *preview = nullptr;
  ToolInteractionState *interaction = nullptr;
  CommandManager *commandManager = nullptr;

  // ── Snapshot values (read-only for most tools) ────────────────────────
  uint32_t fgColor = 0xFF000000;
  uint32_t bgColor = 0xFFFFFFFF;

  // Brush size shorthand — kept for backwards compat; prefer
  // settings->strokeWidth.
  int brushSize = 1;

  ToolSettings *settings = nullptr;

  // ── Write-back pointers ───────────────────────────────────────────────
  // Eyedropper writes the sampled pixel directly into the editor's live
  // color fields via these pointers.
  uint32_t *fgColorOut = nullptr;
  uint32_t *bgColorOut = nullptr;

  // Eyedropper sets this to true after writing fgColorOut / bgColorOut so
  // App::render() knows to push editor → palette instead of the normal
  // palette → editor direction this frame.
  bool *colorSampledOut = nullptr;

  Viewport *viewport = nullptr;

  // SDL_Window* required by SDL_StartTextInput / SDL_StopTextInput (Text tool).
  SDL_Window *window = nullptr;
};
