#pragma once
#include <SDL3/SDL.h>

class Canvas {
  int m_width = 0;
  int m_height = 0;

public:
  SDL_Renderer *m_renderer = nullptr;
  SDL_Surface *m_canvasSurface = nullptr;
  SDL_Texture *m_canvasTexture = nullptr;

  Canvas(SDL_Renderer *renderer, int width, int height);
  ~Canvas() {
    if (this->m_canvasSurface) {
      SDL_DestroySurface(m_canvasSurface);
      this->m_canvasSurface = nullptr;
    }
    if (this->m_canvasTexture) {
      SDL_DestroyTexture(m_canvasTexture);
      this->m_canvasTexture = nullptr;
    }
  }
  void sync() {
    SDL_UpdateTexture(m_canvasTexture, NULL, m_canvasSurface->pixels,
                      m_canvasSurface->pitch);
  }
  void drawPixel(int x, int y, uint32_t color);
  void clear();

  SDL_Surface *surface();
  SDL_Texture *texture();

  int getHeight() const;
  int getWidth() const;
};
