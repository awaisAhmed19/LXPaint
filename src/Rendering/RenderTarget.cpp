
#include "RenderTarget.h"

RenderTarget::RenderTarget(int w, int h) {
  this->m_width = w;
  this->m_height = h;
  this->m_surface = SDL_CreateSurface(this->m_width, this->m_height,
                                      SDL_PIXELFORMAT_ARGB8888);
  if (!this->m_surface) {
    SDL_Log("Failed to create preview surface: %s", SDL_GetError());
  }
}
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

SDL_Surface *RenderTarget::getSurface() { return this->m_surface; }
SDL_Texture *RenderTarget::getTexture() { return this->m_texture; }

int RenderTarget::getWidth() const { return this->m_width; }
int RenderTarget::getHeight() const { return this->m_height; }

void RenderTarget::markDirty() {
  Logger::debug("RenderTarget marked dirty");
  m_dirty = true;
}

bool RenderTarget::isDirty() const { return m_dirty; }
void RenderTarget::clear(uint32_t color) {
  if (!m_surface)
    return;

  SDL_FillSurfaceRect(m_surface, nullptr, color);
  markDirty();
}
void RenderTarget::clearRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  const SDL_PixelFormatDetails *fmt =
      SDL_GetPixelFormatDetails(m_surface->format);

  clear(SDL_MapRGBA(fmt, nullptr, r, g, b, a));
}
