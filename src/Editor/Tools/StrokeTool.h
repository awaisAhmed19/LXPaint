#pragma once

#include "App/Globals.h"
#include "BaseTool.h"

#include <SDL3/SDL.h>

#include <algorithm>

class StrokeTool : public BaseTool {
protected:
  SDL_Surface *m_backupSurface = nullptr;

  SDL_Rect m_strokeBounds{};
  bool m_hasStrokeBounds = false;

protected:
  void beginStrokeBounds(vec2 pos, int radius, int maxW, int maxH) {
    m_hasStrokeBounds = false;

    expandStrokeBounds(pos, radius, maxW, maxH);
  }

  void expandStrokeBounds(vec2 pos, int radius, int maxW, int maxH) {
    SDL_Rect pointRect{.x = (int)pos.x - radius,
                       .y = (int)pos.y - radius,
                       .w = radius * 2 + 1,
                       .h = radius * 2 + 1};

    pointRect.x = std::clamp(pointRect.x, 0, maxW - 1);
    pointRect.y = std::clamp(pointRect.y, 0, maxH - 1);
    pointRect.w = std::min(pointRect.w, maxW - pointRect.x);
    pointRect.h = std::min(pointRect.h, maxH - pointRect.y);

    if (!m_hasStrokeBounds) {
      m_strokeBounds = pointRect;
      m_hasStrokeBounds = true;
      return;
    }

    SDL_Rect unionRect{};
    SDL_GetRectUnion(&m_strokeBounds, &pointRect, &unionRect);
    m_strokeBounds = unionRect;
  }

  void freeBackupSurface() {
    if (m_backupSurface) {
      SDL_DestroySurface(m_backupSurface);
      m_backupSurface = nullptr;
    }
  }

public:
  virtual ~StrokeTool() { freeBackupSurface(); }
};
