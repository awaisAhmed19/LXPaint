#pragma once
#include "../Systems/Logger.h"
#include <SDL3/SDL.h>
class Renderer;
class RenderTarget {
protected:
  SDL_Surface *m_surface = nullptr;
  SDL_Texture *m_texture = nullptr;

  int m_width = 0;
  int m_height = 0;

  bool m_dirty = true;

public:
  friend class Renderer;
  RenderTarget(int w, int h);
  virtual ~RenderTarget();
  void clear(uint32_t color);

  SDL_Surface *getSurface();
  SDL_Texture *getTexture();

  int getWidth() const;
  int getHeight() const;

  void clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  bool isDirty() const;
  void markDirty();
};
