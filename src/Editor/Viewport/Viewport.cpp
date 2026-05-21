#include "Viewport.h"
#include "Systems/Assert.h"
#include <algorithm>
void Viewport::setPan(vec2 pan) { this->m_pan = pan; }

void Viewport::setZoom(float zoom) {
  this->m_zoom = std::clamp(zoom, 0.1f, 16.0f);
  this->m_zoom = zoom;
}

vec2 Viewport::screenToWorld(vec2 screen) const {
  LX_ASSERT(m_zoom > 0.0f, "Viewport zoom invalid");
  return {(screen.x - this->m_pan.x) / this->m_zoom,
          (screen.y - this->m_pan.y) / this->m_zoom};
}
vec2 Viewport::screenToCanvas(vec2 screen,
                              const Transform2D &docTransform) const {

  LX_ASSERT(m_zoom > 0.0f, "Viewport zoom invalid");
  vec2 world = screenToWorld(screen);
  return {world.x - docTransform.position.x, world.y - docTransform.position.y};
}
vec2 Viewport::worldToScreen(vec2 world) const {

  LX_ASSERT(m_zoom > 0.0f, "Viewport zoom invalid");
  return {world.x * this->m_zoom + this->m_pan.x,
          world.y * this->m_zoom + this->m_pan.y};
}

SDL_FRect Viewport::worldRectToScreen(SDL_FRect rect) const {

  LX_ASSERT(m_zoom > 0.0f, "Viewport zoom invalid");
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
SDL_FRect Viewport::getScreenRect() const { return m_screenRect; }
void Viewport::ZoomAt(vec2 screenPoint, float factor) {
  LX_ASSERT(factor > 0.0f, "Invalid zoom factor");
  float oldZoom = m_zoom;
  m_zoom *= factor;
  m_zoom = std::clamp(m_zoom, 0.1f, 16.0f);

  vec2 worldBefore = {(screenPoint.x - m_pan.x) / oldZoom,
                      (screenPoint.y - m_pan.y) / oldZoom};

  m_pan.x = screenPoint.x - worldBefore.x * m_zoom;
  m_pan.y = screenPoint.y - worldBefore.y * m_zoom;
}
