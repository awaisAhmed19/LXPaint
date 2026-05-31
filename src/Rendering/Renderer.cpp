#include "Renderer.h"
#include "Systems/Assert.h"
#include <SDL3/SDL_render.h>

Renderer::Renderer(SDL_Renderer *renderer) { this->m_renderer = renderer; }

SDL_Renderer *Renderer::getSDLRenderer() const { return m_renderer; }

void Renderer::ensureTexture(RenderTarget &target) {

  LX_ASSERT(target.getSurface() != nullptr, "RenderTarget surface missing");
  bool needsRecreate = !target.m_texture ||
                       target.m_textureWidth != target.getWidth() ||
                       target.m_textureHeight != target.getHeight();

  if (!needsRecreate)
    return;

  //   Logger::debug("Recreating render texture");
  if (target.m_texture) {
    SDL_DestroyTexture(target.m_texture);
    target.m_texture = nullptr;
  }

  target.m_texture = SDL_CreateTexture(
      this->m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
      target.getWidth(), target.getHeight());

  LX_ASSERT(target.m_texture != nullptr, "Texture creation failed");

  target.m_textureWidth = target.getWidth();
  target.m_textureHeight = target.getHeight();

  SDL_SetTextureBlendMode(target.m_texture, SDL_BLENDMODE_BLEND);
}

void Renderer::sync(RenderTarget &target) {

  ensureTexture(target);
  LX_ASSERT(target.m_texture != nullptr, "Texture missing after ensureTexture");
  //   Logger::debug("Updating texture from surface");

  bool updated = false;
  if (target.isDirty()) {
    SDL_Rect dirty = target.getDirtyRect();

    uint8_t *pixels = static_cast<uint8_t *>(target.getSurface()->pixels);
    int pitch = target.getSurface()->pitch;
    uint8_t *start = pixels + dirty.y * pitch + dirty.x * 4;

    updated = SDL_UpdateTexture(target.m_texture, &dirty, start, pitch);
    target.clearDirty();
  }

  if (!updated) {
    SDL_Log("UpdateTexture failed: %s", SDL_GetError());
    Logger::err("UpdateTexture Failed");
    return;
  }
}

void Renderer::renderTarget(RenderTarget &target, const Viewport &viewport,
                            const Transform2D &transform) {
  LX_ASSERT(target.getSurface() != nullptr, "RenderTarget surface null");
  if (target.isDirty()) {
    sync(target);
  }

  if (!target.getTexture()) {
    SDL_Log("Texture missing!");
    Logger::err("RenderTarget texture missing after sync");
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
