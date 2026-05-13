#include "Viewport.h"

void Viewport::setPan(vec2 pan) { this->m_pan = pan; }

void Viewport::setZoom(float zoom) { this->m_zoom = zoom; }

vec2 Viewport::screenToWorld(vec2 screen) const {

  return {(screen.x - this->m_pan.x) / this->m_zoom,
          (screen.y - this->m_pan.y) / this->m_zoom};
}

vec2 Viewport::worldToScreen(vec2 world) const {

  return {world.x * this->m_zoom + this->m_pan.x,
          world.y * this->m_zoom + this->m_pan.y};
}

SDL_FRect Viewport::worldRectToScreen(SDL_FRect rect) const {

  return SDL_FRect{rect.x * this->m_zoom + this->m_pan.x,
                   rect.y * this->m_zoom + this->m_pan.y, rect.w * this->m_zoom,
                   rect.h * this->m_zoom};
}

vec2 Viewport::getPan() const { return this->m_pan; }

float Viewport::getZoom() const { return this->m_zoom; }

void Viewport::setScreenRect(SDL_FRect rect) { this->m_screenRect = rect; }

bool Viewport::isVisible(SDL_FRect rect) const {
  SDL_FRect screen = worldRectToScreen(rect);
  return SDL_HasRectIntersectionFloat(&screen, &m_screenRect);
}

SDL_FRect Viewport::getVisibleCanvasBounds() const {
  vec2 topLeft = screenToWorld({m_screenRect.x, m_screenRect.y});
  vec2 bottomRight = screenToWorld(
      {m_screenRect.x + m_screenRect.w, m_screenRect.y + m_screenRect.h});

  return SDL_FRect{
      topLeft.x,
      topLeft.y,
      bottomRight.x - topLeft.x,
      bottomRight.y - topLeft.y,
  };
}

void Viewport::ZoomAt(vec2 screenPoint, float factor) {
  vec2 before = screenToWorld(screenPoint);
  this->m_zoom *= factor;
  vec2 after = screenToWorld(screenPoint);
  this->m_pan.x += (after.x - before.x) * this->m_zoom;
  this->m_pan.y += (after.y - before.y) * this->m_zoom;
}
