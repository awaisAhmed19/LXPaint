#pragma once
#include <SDL3/SDL.h>

class Canvas {
public:
  SDL_Renderer *m_renderer = nullptr;
  SDL_Surface *m_canvasSurface = nullptr;
  SDL_Texture *m_canvasTexture = nullptr;
  SDL_Texture *m_previewTexture = nullptr;

  // Dimensions
  int m_width = 0;
  int m_height = 0;
  Canvas(SDL_Renderer *r, int w, int h) {
    this->m_renderer = r;
    this->m_width = w;
    this->m_height = h;
    m_canvasSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);

    // 2. Setup Hardware Mirror (Main) - STREAMING is for CPU->GPU updates
    m_canvasTexture = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING, w, h);

    // 3. Setup Hardware Preview - TARGET is for Renderer->Texture updates
    m_previewTexture = SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_TARGET, w, h);

    SDL_SetTextureBlendMode(m_previewTexture, SDL_BLENDMODE_BLEND);

    clearAll();
  }

  ~Canvas() {
    SDL_DestroySurface(m_canvasSurface);
    SDL_DestroyTexture(m_canvasTexture);
    SDL_DestroyTexture(m_previewTexture);
  }

  void drawPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= m_canvasSurface->w || y < 0 || y >= m_canvasSurface->h) {
      return;
    }

    uint32_t *pixels = static_cast<uint32_t *>(m_canvasSurface->pixels);

    int pitch_in_pixels = m_canvasSurface->pitch / 4;
    pixels[y * pitch_in_pixels + x] = color;
  }
  // Push pixels from CPU Surface to GPU Texture
  void syncTexture() {
    SDL_UpdateTexture(m_canvasTexture, NULL, m_canvasSurface->pixels,
                      m_canvasSurface->pitch);
  }

  void clearPreview() {
    SDL_SetRenderTarget(m_renderer, m_previewTexture);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0); // Fully Transparent
    SDL_RenderClear(m_renderer);
    SDL_SetRenderTarget(m_renderer, NULL); // Reset target to screen
  }

  void clearAll() {
    SDL_FillSurfaceRect(m_canvasSurface, NULL, 0xFFFFFFFF); // White background
    clearPreview();
    syncTexture();
  }
};
