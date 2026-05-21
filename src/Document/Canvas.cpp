#include "Canvas.h"

#include "Systems/Logger.h"
Canvas::Canvas(int w, int h) : RenderTarget(w, h), m_preview(w, h) {
  clearRGBA(255, 255, 255, 255);
}

void Canvas::resize(int w, int h, const ResizePolicy &policy) {
  int oldW = getWidth();
  int oldH = getHeight();

  RenderTarget newTarget;
  newTarget.allocate(w, h);

  SDL_Rect srcRect = computeSourceRect(oldW, oldH, w, h, policy);
  SDL_Rect dstRect = computeDestinationRect(oldW, oldH, w, h, policy);
  newTarget.blitFrom(*this, &srcRect, &dstRect);

  swapTarget(newTarget);
  // m_preview.allocate(w, h);

  Logger::debug(
      std::format("Canvas final size {}x{}", getWidth(), getHeight()));
  // markDirty();
}

SDL_Rect Canvas::computeDestinationRect(int oldW, int oldH, int newW, int newH,
                                        const ResizePolicy &policy) const {

  SDL_Rect rect{};

  rect.w = std::min(oldW, newW);
  rect.h = std::min(oldH, newH);

  switch (policy.anchor) {

  case ResizeAnchor::TOPLEFT:
    rect.x = 0;
    rect.y = 0;
    break;

  case ResizeAnchor::CENTER:
    rect.x = std::max(0, (newW - oldW) / 2);
    rect.y = std::max(0, (newH - oldH) / 2);
    break;
  }

  return rect;
}
SDL_Rect Canvas::computeSourceRect(int oldW, int oldH, int newW, int newH,
                                   const ResizePolicy &policy) const {

  SDL_Rect rect{};

  rect.w = std::min(oldW, newW);
  rect.h = std::min(oldH, newH);

  switch (policy.anchor) {

  case ResizeAnchor::TOPLEFT:
    rect.x = 0;
    rect.y = 0;
    break;

  case ResizeAnchor::CENTER:
    rect.x = std::max(0, (oldW - newW) / 2);
    rect.y = std::max(0, (oldH - newH) / 2);
    break;
  }

  return rect;
}
