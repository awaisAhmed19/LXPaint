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

  SDL_Rect Boundbox = {0, 0, 0, 0};

  void updateBounds(vec2<float> pos, int brushSize, int maxW, int maxH) {
    int minX = std::min(Boundbox.x, (int)pos.x - brushSize);
    int minY = std::min(Boundbox.y, (int)pos.y - brushSize);
    int maxX = std::max(Boundbox.x + Boundbox.w, (int)pos.x + brushSize);
    int maxY = std::max(Boundbox.y + Boundbox.h, (int)pos.y + brushSize);

    Boundbox.x = std::clamp(minX, 0, maxW);
    Boundbox.y = std::clamp(minY, 0, maxH);
    Boundbox.w = std::clamp(maxX - Boundbox.x, 0, maxW - Boundbox.x);
    Boundbox.h = std::clamp(maxY - Boundbox.y, 0, maxH - Boundbox.y);
  }

  void resetBounds(vec2<float> pos, int brushSize) {
    Boundbox = {(int)pos.x - brushSize, (int)pos.y - brushSize,
                brushSize * 2 + 1, brushSize * 2 + 1};
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
