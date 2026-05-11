#include "Renderer.h"

Renderer::Renderer(SDL_Renderer *renderer) { this->m_renderer = renderer; }

void Renderer::begin() {
  SDL_SetRenderDrawColor(this->m_renderer, 30, 30, 30, 255);
  SDL_RenderClear(this->m_renderer);
}

void Renderer::sync(RenderTarget &target) {
  if (!target.m_texture) {
    target.m_texture = SDL_CreateTexture(
        this->m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
        target.getWidth(), target.getHeight());
    if (!target.m_texture) {
      SDL_Log("Texture creation failed: %s", SDL_GetError());
      return;
    }
    SDL_SetTextureBlendMode(target.m_texture, SDL_BLENDMODE_BLEND);
  }

  SDL_UpdateTexture(target.m_texture, nullptr, target.getSurface()->pixels,
                    target.getSurface()->pitch);
  target.m_dirty = false;
}

void Renderer::renderTarget(RenderTarget &target) {
  if (target.isDirty()) {
    sync(target);
  }
  SDL_FRect dst = {m_pan.x, m_pan.y, target.getWidth() * m_zoom,
                   target.getHeight() * m_zoom};

  SDL_RenderTexture(this->m_renderer, target.getTexture(), nullptr, &dst);
}

void Renderer::end() { SDL_RenderPresent(this->m_renderer); }

vec2 Renderer::screenToCanvas(vec2 screen) const {
  return {(screen.x - m_pan.x) / m_zoom, (screen.y - m_pan.y) / m_zoom};
}
vec2 Renderer::canvasToScreen(vec2 canvas) const {
  return {canvas.x * m_zoom + m_pan.x, canvas.y * m_zoom + m_pan.y};
}

void Renderer::setPan(vec2 pan) { this->m_pan = pan; }

void Renderer::setZoom(float zoom) { this->m_zoom = zoom; }
