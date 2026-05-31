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
  // virtual BaseTool();
  virtual bool usesPreview() const { return false; }
  virtual void onMouseDown(vec2 pos, ToolContext &ctx) = 0;
  virtual void onMouseMove(vec2 pos, ToolContext &ctx) = 0;
  virtual std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) = 0;
};
