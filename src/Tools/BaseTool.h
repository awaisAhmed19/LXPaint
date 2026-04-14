#pragma once
#include "../core/Command.h"
#include "Globals.h"
class Canvas;
class BaseTool {
public:
  virtual ~BaseTool() {}
  virtual void onMouseDown(vec2 pos, Canvas &canvas) = 0;
  virtual void onMouseMove(vec2 pos, Canvas &canvas) = 0;
  virtual Command *onMouseUp(vec2 pos, Canvas &canvas) = 0;
};
