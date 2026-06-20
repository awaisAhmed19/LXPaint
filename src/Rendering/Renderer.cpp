#include "Renderer.h"
#include "Systems/Assert.h"
#include <SDL3/SDL_render.h>
namespace {

void logRenderTarget(const Viewport &viewport, const RenderTarget &target,
                     const Transform2D &transform, const SDL_FRect &dst) {

  SDL_FRect vp = viewport.getScreenRect();

  std::cout << "\n================ RENDER TARGET ================\n";

  std::cout << "Viewport Rect\n";
  std::cout << "  X      : " << vp.x << '\n';
  std::cout << "  Y      : " << vp.y << '\n';
  std::cout << "  Width  : " << vp.w << '\n';
  std::cout << "  Height : " << vp.h << "\n\n";

  std::cout << "Canvas\n";
  std::cout << "  Width  : " << target.getWidth() << '\n';
  std::cout << "  Height : " << target.getHeight() << "\n\n";

  std::cout << "World Transform\n";
  std::cout << "  Position X : " << transform.position.x << '\n';
  std::cout << "  Position Y : " << transform.position.y << "\n\n";

  std::cout << "Destination Rect\n";
  std::cout << "  X      : " << dst.x << '\n';
  std::cout << "  Y      : " << dst.y << '\n';
  std::cout << "  Width  : " << dst.w << '\n';
  std::cout << "  Height : " << dst.h << "\n\n";

  std::cout << "Texture Pointer : " << target.getTexture() << '\n';

  std::cout << "===============================================\n";
}

} // namespace
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

  if (!target.isDirty())
    return;
  SDL_Rect dirty = target.getDirtyRect();
  if (dirty.w <= 0 || dirty.h <= 0) {
    dirty = {0, 0, target.getWidth(), target.getHeight()};
  }
  uint8_t *pixels = static_cast<uint8_t *>(target.getSurface()->pixels);
  int pitch = target.getSurface()->pitch;
  uint8_t *start = pixels + dirty.y * pitch + dirty.x * 4;

  if (!SDL_UpdateTexture(target.m_texture, &dirty, start, pitch)) {
    SDL_Log("UpdateTexture failed: %s", SDL_GetError());
    Logger::err("UpdateTexture Failed");
    return;
  }
  target.clearDirty();
}
/*void Renderer::beginViewport(const Viewport &viewport) {
  SDL_FRect vp = viewport.getScreenRect();

  SDL_Rect clip = {static_cast<int>(vp.x), static_cast<int>(vp.y),
                   static_cast<int>(vp.w), static_cast<int>(vp.h)};

  std::cout << "\n========== BEGIN VIEWPORT ==========\n";
  std::cout << "Clip Rect\n";
  std::cout << "  X      : " << clip.x << '\n';
  std::cout << "  Y      : " << clip.y << '\n';
  std::cout << "  Width  : " << clip.w << '\n';
  std::cout << "  Height : " << clip.h << '\n';
  std::cout << "====================================\n";

  SDL_SetRenderClipRect(m_renderer, &clip);
}*/
void Renderer::beginViewport(const Viewport &) {}
void Renderer::endViewport() {}
// void Renderer::endViewport() { SDL_SetRenderClipRect(m_renderer, nullptr); }
/*Even better

    Eventually,
    we wouldn't clip per target.

    we would clip once for the whole viewport.

    renderer.beginViewport(viewport);

renderer.drawCheckerboard(...);
renderer.renderTarget(canvas, viewport, transform);
renderer.renderTarget(preview, viewport, transform);
renderer.drawSelection(...);
renderer.drawGrid(...);

renderer.endViewport();
*/
void Renderer::renderTarget(RenderTarget &target, const Viewport &viewport,
                            const Transform2D &transform) {
  LX_ASSERT(target.getSurface() != nullptr, "RenderTarget surface null");

  if (target.isDirty()) {
    sync(target);
  }

  if (!target.getTexture()) {
    Logger::err("RenderTarget texture missing after sync");
    return;
  }

  SDL_FRect worldRect = {transform.position.x, transform.position.y,
                         static_cast<float>(target.getWidth()),
                         static_cast<float>(target.getHeight())};

  SDL_FRect dst = viewport.worldRectToScreen(worldRect);

  if (!SDL_RenderTexture(m_renderer, target.getTexture(), nullptr, &dst)) {
    Logger::err(std::format("SDL_RenderTexture failed: {}", SDL_GetError()));
  }
}
