#pragma once
#include "App/Globals.h"
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
  SDL_Rect m_dirtyRect{0, 0, 0, 0};
  enum KIND { PREVIEW, CANVAS, LAYER };
  bool m_dirty = false;

public:
  friend class Renderer;
  enum class FillColor { TRANSPARENT, WHITE, BLACK };
  SDL_Rect m_boundingBox = {0, 0, 0, 0};

  void allocate(int w, int h, FillColor fill = FillColor::TRANSPARENT);
  RenderTarget(int w, int h);
  RenderTarget();
  virtual ~RenderTarget();
  void clear(uint32_t color);

  SDL_Surface *getSurface();
  SDL_Texture *getTexture();

  void resize(int w, int h);

  int getWidth() const;
  int getHeight() const;
  SDL_Rect getDirtyRect() const;
  void clearDirty();
  void swapTarget(RenderTarget &other);
  SDL_Rect updateBounds(vec2 pos, int brushSize, int maxW, int maxH);
  void resetBounds(vec2 pos, int brushSize);
  void invalidateRect(const SDL_Rect &rect);
  void blitFrom(const RenderTarget &src, const SDL_Rect *srcRect,
                const SDL_Rect *dstRect);

  void clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  bool isDirty() const;
  void markDirty();
};
