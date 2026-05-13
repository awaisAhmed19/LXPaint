#include "Renderer.h"

Renderer::Renderer(SDL_Renderer *renderer) { this->m_renderer = renderer; }

void Renderer::sync(RenderTarget &target) {
  SDL_Log("SYNC START");

  if (!target.m_texture) {

    SDL_Log("CREATING TEXTURE");

    target.m_texture = SDL_CreateTexture(
        this->m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        target.getWidth(), target.getHeight());

    if (!target.m_texture) {
      SDL_Log("Texture creation failed: %s", SDL_GetError());
      return;
    }

    SDL_Log("TEXTURE CREATED");

    SDL_SetTextureBlendMode(target.m_texture, SDL_BLENDMODE_BLEND);
  }

  SDL_Log("UPDATING TEXTURE");

  if (!SDL_UpdateTexture(target.m_texture, nullptr, target.getSurface()->pixels,
                         target.getSurface()->pitch)) {
    SDL_Log("UpdateTexture failed: %s", SDL_GetError());
    return;
  }

  SDL_Log("TEXTURE UPDATED");

  target.m_dirty = false;
}
void Renderer::renderTarget(RenderTarget &target, const Viewport &viewport,
                            const Transform2D &transform) {
  if (target.isDirty()) {
    sync(target);
  }

  if (!target.getTexture()) {
    SDL_Log("Texture missing!");
    return;
  }
  SDL_FRect mainRect = {transform.position.x, transform.position.y,
                        (float)target.getWidth(), (float)target.getHeight()};

  SDL_FRect dst = viewport.worldRectToScreen(mainRect);

  if (!SDL_RenderTexture(this->m_renderer, target.getTexture(), nullptr,
                         &dst)) {
    SDL_Log("RenderTexture failed: %s", SDL_GetError());
  }
}
