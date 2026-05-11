#pragma once
#include "../App/Globals.h"
#include "../Commands/Command.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>

struct ToolContext;

class BaseTool {
protected:
  SDL_Surface *m_snapshotSurface = nullptr;
  SDL_Rect m_boundingBox = {0, 0, 0, 0};
  void freeSnapshot() {
    if (m_snapshotSurface) {
      SDL_DestroySurface(m_snapshotSurface);
      m_snapshotSurface = nullptr;
    }
  }

  void updateBounds(vec2 pos, int brushSize, int maxW, int maxH) {
    int minX = std::min(m_boundingBox.x, (int)pos.x - brushSize);
    int minY = std::min(m_boundingBox.y, (int)pos.y - brushSize);
    int maxX =
        std::max(m_boundingBox.x + m_boundingBox.w, (int)pos.x + brushSize);
    int maxY =
        std::max(m_boundingBox.y + m_boundingBox.h, (int)pos.y + brushSize);

    int newMinX = std::clamp(minX, 0, maxW - 1);
    int newMinY = std::clamp(minY, 0, maxH - 1);
    int newMaxX = std::clamp(maxX, 0, maxW - 1);
    int newMaxY = std::clamp(maxY, 0, maxH - 1);

    m_boundingBox.x = newMinX;
    m_boundingBox.y = newMinY;
    m_boundingBox.w = std::max(0, newMaxX - newMinX + 1);
    m_boundingBox.h = std::max(0, newMaxY - newMinY + 1);
  }

  void resetBounds(vec2 pos, int brushSize) {
    m_boundingBox = {(int)pos.x - brushSize, (int)pos.y - brushSize,
                     brushSize * 2 + 1, brushSize * 2 + 1};
  }

public:
  virtual ~BaseTool() { freeSnapshot(); }
  virtual void deactivate() { freeSnapshot(); }
  // virtual BaseTool();
  virtual void onMouseDown(vec2 pos, ToolContext &ctx) = 0;
  virtual void onMouseMove(vec2 pos, ToolContext &ctx) = 0;
  virtual std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) = 0;
};
