#pragma once
#include "App/Globals.h"
#include "Editor/Commands/Command.h"
#include <SDL3/SDL.h>
#include <memory>

struct ToolContext;

class ClickTool {
public:
  virtual ~ClickTool() {}
  virtual void deactivate() {}
  virtual bool usesPreview() const { return false; }
  virtual std::unique_ptr<Command> onMouseClick(vec2 pos, ToolContext &ctx) = 0;
};
