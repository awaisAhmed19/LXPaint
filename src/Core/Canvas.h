#pragma once
#include <SDL3/SDL.h>

class Canvas {
  int m_width = 0;
  int m_height = 0;

public:
  SDL_Renderer *m_renderer = nullptr;
  SDL_Surface *m_canvasSurface = nullptr;
  SDL_Texture *m_canvasTexture = nullptr;
  SDL_Texture *m_previewTexture = nullptr;

  // Dimensions
  Canvas(SDL_Renderer *r, int w, int h);
  ~Canvas() {
    SDL_DestroySurface(m_canvasSurface);
    SDL_DestroyTexture(m_canvasTexture);
    SDL_DestroyTexture(m_previewTexture);
  }
  void syncTexture() {
    SDL_UpdateTexture(m_canvasTexture, NULL, m_canvasSurface->pixels,
                      m_canvasSurface->pitch);
  }
  void drawPixel(int x, int y, uint32_t color);
  void clearPreview();
  void clearAll();
  int getHeight();
  int getWidth();
};
