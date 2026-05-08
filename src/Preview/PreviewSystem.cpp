#include "PreviewSystem.h"

PreviewSystem::PreviewSystem(SDL_Renderer *renderer, int w, int h, ) {
  this->m_width = w;
  this->m_height = h;
  this->renderer = renderer;
  this->m_currentColor = color;
  this->m_surface = SDL_CreateSurface(this->m_width, this->m_height,
                                      SDL_PIXELFORMAT_ARGB8888);
  if (!this->m_surface) {
    SDL_Log("Failed to create preview surface: %s", SDL_GetError());
  }

  this->m_previewTex = SDL_CreateTexture(
      this->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
      this->m_width, this->m_height);

  if (!this->m_previewTex) {
    SDL_Log("Failed to create preview texture: %s", SDL_GetError());
    SDL_DestroySurface(this->m_surface);
    this->m_surface = nullptr;
    return;
  }

  SDL_SetTextureBlendMode(this->m_previewTex, SDL_BLENDMODE_BLEND);
  clear();
}
PreviewSystem::~PreviewSystem() {
  if (this->m_surface) {
    SDL_DestroySurface(this->m_surface);
    this->m_surface = nullptr;
  }

  if (this->m_previewTex) {
    SDL_DestroyTexture(this->m_previewTex);
    this->m_previewTex = nullptr;
  }
}

SDL_Surface *PreviewSystem::surface() { return this->m_surface; }
SDL_Texture *PreviewSystem::texture() { return this->m_previewTex; }

int PreviewSystem::getWidth() { return this->m_width; }
int PreviewSystem::getHeight() { return this->m_height; }

// void PreviewSystem::clearRenderer() {
//   SDL_setRenderTarget(m_renderer, m_previewTex);
//   SDL_setRenderDrawColor(m_renderer, 0, 0, 0, 0);
//   SDL_RenderClear(m_renderer);
//   SDL_setRenderTarget(m_renderer, NULL);
// }

void PreviewSystem::sync() {
  SDL_UpdateTexture(m_previewTex, NULL, m_surface->pixels, m_surface->pitch);
}

void PreviewSystem::clear() {
  SDL_FillSurfaceRect(m_surface, NULL,
                      SDL_MapRGB(this->m_surface->format, 0, 0, 0, 0));
  sync();
}
