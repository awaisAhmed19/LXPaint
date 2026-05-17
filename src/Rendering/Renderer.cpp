#include "Renderer.h"

Renderer::Renderer(SDL_Renderer *renderer) { this->m_renderer = renderer; }

void Renderer::sync(RenderTarget &target) {
  Logger::debug("Renderer::sync START");
  if (!target.m_texture) {

    target.m_texture = SDL_CreateTexture(
        this->m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        target.getWidth(), target.getHeight());

    if (!target.m_texture) {
      SDL_Log("Texture creation failed: %s", SDL_GetError());
      return;
    }

    Logger::debug("Texture created");

    SDL_SetTextureBlendMode(target.m_texture, SDL_BLENDMODE_BLEND);
  }

  Logger::debug("Updating texture from surface");
  bool updated =
      SDL_UpdateTexture(target.m_texture, nullptr, target.getSurface()->pixels,
                        target.getSurface()->pitch);

  if (!updated) {
    SDL_Log("UpdateTexture failed: %s", SDL_GetError());
    return;
  }

  Logger::debug("Texture updated successfully");
  target.m_dirty = false;
  Logger::debug("Dirty flag reset");
}

void Renderer::renderTarget(RenderTarget &target, const Viewport &viewport,
                            const Transform2D &transform) {
  Logger::debug(std::format("renderTarget dirty={} texture={}",
                            target.isDirty(), (void *)target.getTexture()));
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

  bool rendered =
      SDL_RenderTexture(this->m_renderer, target.getTexture(), nullptr, &dst);

  if (!rendered) {
    SDL_Log("RenderTexture failed: %s", SDL_GetError());
  }
}
