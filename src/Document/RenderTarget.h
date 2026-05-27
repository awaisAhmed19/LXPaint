#pragma once
#include <SDL3/SDL.h>

class Renderer;
class RenderTarget {
protected:
  SDL_Surface *m_surface = nullptr;
  SDL_Texture *m_texture = nullptr;
  int m_textureWidth = 0;
  int m_textureHeight = 0;
  int m_width = 0;
  int m_height = 0;
  enum KIND { PREVIEW, CANVAS, LAYER };
  bool m_dirty = true;

public:
  friend class Renderer;

  RenderTarget(int w, int h);
  RenderTarget();
  virtual ~RenderTarget();
  void clear(uint32_t color);

  SDL_Surface *getSurface();
  SDL_Texture *getTexture();

  void resize(int w, int h);

  int getWidth() const;
  int getHeight() const;

  void swapTarget(RenderTarget &other);

  void allocate(int w, int h);

  void blitFrom(const RenderTarget &src, const SDL_Rect *srcRect,
                const SDL_Rect *dstRect);

  void clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  bool isDirty() const;
  void markDirty();
};
