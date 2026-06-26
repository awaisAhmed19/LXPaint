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
/*
void RenderTarget::updateBounds(const SDL_Rect &rect) {
  m_dirty = SDL_GetRectUnion(&m_dirty, &rect);
}

void RenderTarget::resetBounds() { m_dirty = {0, 0, 0, 0}; }
*/
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

SDL_Surface *RenderTarget::getSurface() const { return this->m_surface; }
SDL_Texture *RenderTarget::getTexture() const { return this->m_texture; }

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

void RenderTarget::resize(int w, int h, const ResizePolicy &policy) {
  int oldW = getWidth();
  int oldH = getHeight();

  RenderTarget newTarget;

  FillColor fill = (policy.fill == ResizeFill::BACKGROUNDCOLOR)
                       ? FillColor::WHITE
                       : FillColor::TRANSPARENT;

  newTarget.allocate(w, h, fill);

  SDL_Rect src = computeSourceRect(oldW, oldH, w, h, policy);
  SDL_Rect dst = computeDestinationRect(oldW, oldH, w, h, policy);

  newTarget.blitFrom(*this, &src, &dst);

  swapTarget(newTarget);

  markDirty();
}
SDL_Rect
RenderTarget::computeDestinationRect(int oldW, int oldH, int newW, int newH,
                                     const ResizePolicy &policy) const {

  SDL_Rect rect{};
  rect.w = std::min(oldW, newW);
  rect.h = std::min(oldH, newH);

  switch (policy.anchor) {
  case ResizeAnchor::TOPLEFT:
    rect.x = 0;
    rect.y = 0;
    break;

  case ResizeAnchor::CENTER:
    rect.x = std::max(0, (newW - oldW) / 2);
    rect.y = std::max(0, (newH - oldH) / 2);
    break;
  }

  return rect;
}
SDL_Rect RenderTarget::computeSourceRect(int oldW, int oldH, int newW, int newH,
                                         const ResizePolicy &policy) const {

  SDL_Rect rect{};
  rect.w = std::min(oldW, newW);
  rect.h = std::min(oldH, newH);

  switch (policy.anchor) {
  case ResizeAnchor::TOPLEFT:
    rect.x = 0;
    rect.y = 0;
    break;

  case ResizeAnchor::CENTER:
    rect.x = std::max(0, (oldW - newW) / 2);
    rect.y = std::max(0, (oldH - newH) / 2);
    break;
  }

  return rect;
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
