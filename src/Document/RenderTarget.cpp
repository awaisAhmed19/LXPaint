#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_surface.h>

#include <algorithm>

#include "RenderTarget.h"

#include "Systems/Assert.h"

RenderTarget::RenderTarget(int w, int h) {
  this->m_width = w;
  this->m_height = h;
  this->m_surface = SDL_CreateSurface(this->m_width, this->m_height,
                                      SDL_PIXELFORMAT_ARGB8888);
  LX_ASSERT(this->m_surface != nullptr, "RenderTarget surface creation failed");
  if (!this->m_surface) {
    SDL_Log("Failed to create preview surface: %s", SDL_GetError());
  }
}

RenderTarget::RenderTarget()
    : m_surface(nullptr), m_texture(nullptr), m_textureWidth(0),
      m_textureHeight(0), m_width(0), m_height(0), m_dirty(false) {}

RenderTarget::~RenderTarget() {
  if (this->m_surface) {
    SDL_DestroySurface(this->m_surface);
    this->m_surface = nullptr;
  }

  if (this->m_texture) {
    SDL_DestroyTexture(this->m_texture);
    this->m_texture = nullptr;
  }
}
SDL_Rect RenderTarget::updateBounds(vec2 pos, int brushSize, int maxW,
                                    int maxH) {
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
  return m_boundingBox;
}

// void RenderTarget::resetBounds(vec2 pos, int brushSize) {
//   m_boundingBox = {(int)pos.x - brushSize, (int)pos.y - brushSize,
//                    brushSize * 2 + 1, brushSize * 2 + 1};
// }

void RenderTarget::invalidateRect(const SDL_Rect &rect) {
  if (!m_dirty) {
    m_dirtyRect = rect;
    m_dirty = true;
    return;
  }

  SDL_Rect result;

  SDL_GetRectUnion(&m_dirtyRect, &rect, &result);

  m_dirtyRect = result;
}

SDL_Surface *RenderTarget::getSurface() { return this->m_surface; }
SDL_Texture *RenderTarget::getTexture() { return this->m_texture; }

int RenderTarget::getWidth() const { return this->m_width; }
int RenderTarget::getHeight() const { return this->m_height; }
SDL_Rect RenderTarget::getDirtyRect() const { return this->m_dirtyRect; }
void RenderTarget::clearDirty() {
  m_dirty = false;
  m_dirtyRect = {0, 0, 0, 0};
}
void RenderTarget::allocate(int w, int h, FillColor fill) {

  LX_ASSERT(w > 0 && h > 0, "Invalid RenderTarget allocation");

  Logger::debug(std::format("Allocating RenderTarget {}x{} -> {}x{}", m_width,
                            m_height, w, h));

  SDL_Surface *newSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);

  LX_ASSERT(newSurface != nullptr, "Failed to allocate RenderTarget surface");

  /*
      Initialize surface memory.

      Uninitialized pixel memory can contain random garbage,
      especially after canvas expansion.
  */

  uint32_t fillcolor;
  switch (fill) {
  case FillColor::TRANSPARENT:
    fillcolor = SDL_MapSurfaceRGBA(newSurface, 0, 0, 0, 0);
    break;
  case FillColor::WHITE:
    fillcolor = SDL_MapSurfaceRGBA(newSurface, 255, 255, 255, 255);
    break;
  case FillColor::BLACK:
    fillcolor = SDL_MapSurfaceRGBA(newSurface, 0, 0, 0, 255);
    break;
  }
  SDL_FillSurfaceRect(newSurface, nullptr, fillcolor);

  /*
      Destroy previous surface.
  */

  if (m_surface) {
    SDL_DestroySurface(m_surface);
  }

  m_surface = newSurface;

  m_width = w;
  m_height = h;

  /*
      Existing GPU texture is now invalid.
      Force lazy recreation during next sync().
  */

  if (m_texture) {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }

  m_textureWidth = 0;
  m_textureHeight = 0;

  markDirty();
}

void RenderTarget::blitFrom(const RenderTarget &src, const SDL_Rect *srcRect,
                            const SDL_Rect *dstRect) {
  LX_ASSERT(m_surface != nullptr, "Destination surface is null");
  LX_ASSERT(src.m_surface != nullptr, "Source surface is null");

  bool success = SDL_BlitSurface(src.m_surface, srcRect, m_surface, dstRect);
  LX_ASSERT(success, "Failed to blit RenderTarget");

  markDirty();
}

void RenderTarget::resize(int w, int h) {
  LX_ASSERT(w > 0 && h > 0, "Invalid RenderTarget resize");
  Logger::debug(std::format("Resizing RenderTarget {}x{} -> {}x{}", m_width,
                            m_height, w, h));
  SDL_Surface *newSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);
  LX_ASSERT(newSurface != nullptr, "Failed to create resized surface");

  SDL_FillSurfaceRect(newSurface, nullptr,
                      SDL_MapSurfaceRGBA(newSurface, 255, 255, 255, 255));

  if (m_surface) {
    SDL_Rect src = {0, 0, std::min(m_width, w), std::min(m_height, h)};
    SDL_BlitSurface(m_surface, &src, newSurface, &src);
    SDL_DestroySurface(m_surface);
  }

  m_surface = newSurface;

  m_width = w;
  m_height = h;

  if (m_texture) {
    SDL_DestroyTexture(m_texture);
    m_texture = nullptr;
  }

  m_textureWidth = 0;
  m_textureHeight = 0;

  markDirty();
}

void RenderTarget::markDirty() {
  //   Logger::debug("RenderTarget marked dirty");
  m_dirty = true;
  m_dirtyRect = {0, 0, m_width, m_height};
}

void RenderTarget::swapTarget(RenderTarget &other) {
  std::swap(m_surface, other.m_surface);
  std::swap(m_texture, other.m_texture);
  std::swap(m_width, other.m_width);
  std::swap(m_height, other.m_height);
  std::swap(m_textureWidth, other.m_textureWidth);
  std::swap(m_textureHeight, other.m_textureHeight);
  m_dirty = true;
  // std::swap(m_dirty, other.m_dirty);
  Logger::debug(std::format("Swapping targets {}x{} <-> {}x{}", m_width,
                            m_height, other.m_width, other.m_height));
}

bool RenderTarget::isDirty() const { return m_dirty; }

void RenderTarget::clear(uint32_t color) {
  LX_ASSERT(m_surface != nullptr, "clear called with null surface");
  if (!m_surface)
    return;

  SDL_FillSurfaceRect(m_surface, nullptr, color);
  markDirty();
}

void RenderTarget::clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  LX_ASSERT(m_surface != nullptr, "clearRGBA null surface");
  const SDL_PixelFormatDetails *fmt =
      SDL_GetPixelFormatDetails(m_surface->format);

  clear(SDL_MapRGBA(fmt, nullptr, r, g, b, a));
}
