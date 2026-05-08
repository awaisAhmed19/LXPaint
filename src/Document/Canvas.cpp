#include "./Canvas.h"

Canvas ::Canvas(SDL_Renderer *renderer, int width, int height) {
  this->m_renderer = renderer;
  this->m_width = width;
  this->m_height = height;
  this->m_canvasSurface =
      SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
  if (!this->m_canvasSurface) {
    SDL_Log("Failed to create preview surface: %s", SDL_GetError());
  }

  this->m_canvasTexture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING, width, height);

  if (!this->m_canvasTexture) {
    SDL_Log("Failed to create preview texture: %s", SDL_GetError());
    SDL_DestroySurface(this->m_canvasSurface);
    this->m_canvasSurface = nullptr;
    return;
  }
  SDL_SetTextureBlendMode(this->m_canvasTexture, SDL_BLENDMODE_BLEND);

  clear();
}

int Canvas ::getWidth() const { return this->m_width; }
int Canvas ::getHeight() const { return this->m_height; }

SDL_Surface *Canvas ::surface() { return this->m_canvasSurface; }
SDL_Texture *Canvas ::texture() { return this->m_canvasTexture; }

void Canvas ::drawPixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= this->m_canvasSurface->w || y < 0 ||
      y >= this->m_canvasSurface->h) {
    return;
  }
  uint32_t *pixels = static_cast<uint32_t *>(this->m_canvasSurface->pixels);

  int pitch_in_pixels = this->m_canvasSurface->pitch / 4;
  pixels[y * pitch_in_pixels + x] = color;
}

void Canvas::clear() {
  SDL_FillSurfaceRect(this->m_canvasSurface, NULL,
                      SDL_MapRGB(this->m_canvasSurface->format, 255, 255, 255,
                                 255)); // White background
  sync();
}
