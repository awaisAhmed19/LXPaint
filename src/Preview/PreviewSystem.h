#pragma once
// #include "../Globals"
#include <SDL3/SDL.h>
class PreviewSystem {
private:
  // vec2 m_previewPos;
  int m_width = 0;
  int m_height = 0;
  SDL_Renderer *m_renderer = nullptr;
  SDL_Surface *m_surface = nullptr;
  SDL_Texture *m_previewTex = nullptr;

public:
  PreviewSystem(SDL_Renderer *renderer, int width, int height);
  ~PreviewSystem();
  SDL_Surface *getSurface();
  SDL_Texture *getTexture();
  void sync();
  int getWidth();
  int getHeight();
  void clear();
};
