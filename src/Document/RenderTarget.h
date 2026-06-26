#pragma once
#include "App/Globals.h"
#include <SDL3/SDL.h>
struct ResizePolicy;
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

protected:
  SDL_Rect computeSourceRect(int oldW, int oldH, int newW, int newH,
                             const ResizePolicy &policy) const;

  SDL_Rect computeDestinationRect(int oldW, int oldH, int newW, int newH,
                                  const ResizePolicy &policy) const;

public:
  friend class Renderer;
  enum class FillColor { TRANSPARENT, WHITE, BLACK };

  void allocate(int w, int h, FillColor fill = FillColor::TRANSPARENT);
  RenderTarget(int w, int h);
  RenderTarget();
  virtual ~RenderTarget();
  void clear(uint32_t color);

  SDL_Surface *getSurface() const;
  SDL_Texture *getTexture() const;

  void resize(int w, int h, const ResizePolicy &policy);

  int getWidth() const;
  int getHeight() const;
  SDL_Rect getDirtyRect() const;
  void clearDirty();
  void swapTarget(RenderTarget &other);
  void invalidateRect(const SDL_Rect &rect);
  void blitFrom(const RenderTarget &src, const SDL_Rect *srcRect,
                const SDL_Rect *dstRect);

  void clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  bool isDirty() const;
  void markDirty();
  void invertColors();
  void flipHorizontal();
  void flipVertical();

  // 90° rotation changes width/height, so these reallocate m_surface
  // (same pattern as resize()/allocate()) rather than mutating in place.
  void rotate90CW();
  void rotate90CCW();
};
