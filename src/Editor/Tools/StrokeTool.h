#pragma once

#include "BaseTool.h"
#include <SDL3/SDL.h>

class StrokeTool : public BaseTool {
protected:
  SDL_Surface *m_backupSurface = nullptr;

public:
  virtual ~StrokeTool() {
    if (m_backupSurface)
      SDL_DestroySurface(m_backupSurface);
  }
};
