#pragma once
#include "../Commands/DrawCommand.h"
#include "../Core/Canvas.h"
#include "../Core/Command.h"
#include "../Core/Logger.h"
#include "../Core/Profiler.h"
#include "../Core/Renderer.h"
#include "Globals.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <vector>
class Canvas;
class BaseTool {
protected:
  SDL_Surface *currentSnapshot = nullptr;
  bool isDrawing = false;
  void freeSnapshot() {
    if (currentSnapshot) {
      SDL_DestroySurface(currentSnapshot);
      currentSnapshot = nullptr;
    }
  }

public:
  virtual ~BaseTool() { freeSnapshot(); }
  virtual void deactivate() {
    isDrawing = false;
    freeSnapshot();
  }

  virtual void onMouseDown(vec2<float> pos, Canvas &canvas) = 0;
  virtual void onMouseMove(vec2<float> pos, Canvas &canvas) = 0;
  virtual Command *onMouseUp(vec2<float> pos, Canvas &canvas) = 0;
};
