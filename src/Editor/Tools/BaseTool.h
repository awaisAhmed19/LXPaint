#pragma once
#include "App/Globals.h"
#include "Editor/Commands/Command.h"
#include <SDL3/SDL.h>
#include <memory>

struct ToolContext;

class BaseTool {
protected:
  SDL_Surface *m_snapshotSurface = nullptr;

  void freeSnapshot() {
    if (m_snapshotSurface) {
      SDL_DestroySurface(m_snapshotSurface);
      m_snapshotSurface = nullptr;
    }
  }

public:
  virtual ~BaseTool() { freeSnapshot(); }
  virtual void deactivate() { freeSnapshot(); }
  virtual bool usesPreview() const { return false; }
  virtual void onMouseDown(vec2 pos, ToolContext &ctx) = 0;
  virtual void onMouseMove(vec2 pos, ToolContext &ctx) = 0;
  virtual std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) = 0;

  // Additive hook: most tools don't need keyboard input, so the default is
  // a no-op and every existing tool (Pencil, Line, Rect, ...) compiles
  // unchanged. Polygon overrides this for Escape-to-cancel; Text overrides
  // it for the full text-editing keyset. Return true if the key was
  // consumed (so Editor doesn't also treat it as a global shortcut).
  virtual bool onKeyDown(SDL_Scancode scancode, ToolContext &ctx) {
    (void)scancode;
    (void)ctx;
    return false;
  }

  // Additive hook for text entry (Text tool). SDL3 delivers typed unicode
  // text via SDL_EVENT_TEXT_INPUT separately from key events; default is a
  // no-op for every tool except Text.
  virtual bool onTextInput(const char *text, ToolContext &ctx) {
    (void)text;
    (void)ctx;
    return false;
  }

  // Default false — most tools (Pencil, Line, Rect, ...) never need
  // onMouseMove called while no mouse button is held, since they only
  // ever draw during a single press-drag-release. Polygon overrides this
  // to true while actively placing vertices, so Editor::handleEvent knows
  // to forward hover-only mouse moves to it (needed for the rubber-band
  // edge to track the cursor between clicks, since there is no drag).
  virtual bool wantsHoverMoves() const { return false; }
};
