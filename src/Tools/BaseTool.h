#pragma once
#include "../Commands/Command.h"
#include "../Commands/DrawCommand.h"
#include "../Document/Canvas.h"
#include "../Globals.h"
#include "../Renderer/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <memory>
#include <vector>
class PreviewSystem;
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

  void updateBounds(vec2 pos, int brushSize, int maxW, int maxH) {
    int minX = std::min(Boundbox.x, (int)pos.x - brushSize);
    int minY = std::min(Boundbox.y, (int)pos.y - brushSize);
    int maxX = std::max(Boundbox.x + Boundbox.w, (int)pos.x + brushSize);
    int maxY = std::max(Boundbox.y + Boundbox.h, (int)pos.y + brushSize);

    int newMinX = std::clamp(minX, 0, maxW - 1);
    int newMinY = std::clamp(minY, 0, maxH - 1);
    int newMaxX = std::clamp(maxX, 0, maxW - 1);
    int newMaxY = std::clamp(maxY, 0, maxH - 1);

    Boundbox.x = newMinX;
    Boundbox.y = newMinY;
    Boundbox.w = std::max(0, newMaxX - newMinX + 1);
    Boundbox.h = std::max(0, newMaxY - newMinY + 1);
  }

  void resetBounds(vec2 pos, int brushSize) {
    Boundbox = {(int)pos.x - brushSize, (int)pos.y - brushSize,
                brushSize * 2 + 1, brushSize * 2 + 1};
  }

public:
  virtual ~BaseTool() { freeSnapshot(); }
  virtual void deactivate() {
    isDrawing = false;
    freeSnapshot();
  }
  // virtual BaseTool();
  virtual void onMouseDown(vec2 pos, SDL_Surface *surface) = 0;
  virtual void onMouseMove(vec2 pos, SDL_Surface *surface,
                           PreviewSystem *ps) = 0;
  virtual std::unique_ptr<Command> onMouseUp(vec2 pos, SDL_Surface *surface,
                                             PreviewSystem *ps) = 0;
};
