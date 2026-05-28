#include <algorithm>

#include "Viewport.h"

#include "Systems/Assert.h"

#define MIN_ZOOM 0.1f
#define MAX_ZOOM 16.0f
#define MARGIN 0.95f
void Viewport::setPan(vec2 pan) {
  this->m_pan = pan;
  clampPan();
}

void Viewport::setZoom(float zoom) {
  this->m_zoom = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
  // this->m_zoom = zoom;
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

SDL_FRect Viewport::getCanvasBoundsScreen() const {
  SDL_FRect canvasWorldRect = {0.0f, 0.0f, (float)m_canvasWidth,
                               (float)m_canvasHeight};
  return worldRectToScreen(canvasWorldRect);
}
vec2 Viewport::getCanvasTopLeftScreen(const Transform2D &docTransform) const {
  vec2 topleft = {0.0f, 0.0f};
  vec2 topleftworld = {topleft.x + docTransform.position.x,
                       topleft.y + docTransform.position.y};
  return worldToScreen(topleftworld);
}
vec2 Viewport::getCanvasTopRightScreen(const Transform2D &docTransform) const {
  vec2 topright = {(float)(m_canvasWidth - 1), 0.0f};
  vec2 toprightworld = {topright.x + docTransform.position.x,
                        topright.y + docTransform.position.y};
  return worldToScreen(toprightworld);
}
vec2 Viewport::getCanvasBottomLeftScreen(
    const Transform2D &docTransform) const {

  vec2 bottomleft = {0.0f, (float)(m_canvasHeight - 1)};
  vec2 bottomleftworld = {bottomleft.x + docTransform.position.x,
                          bottomleft.y + docTransform.position.y};
  return worldToScreen(bottomleftworld);
}
vec2 Viewport::getCanvasBottomRightScreen(
    const Transform2D &docTransform) const {

  vec2 bottomright = {(float)(m_canvasWidth - 1), (float)(m_canvasHeight - 1)};
  vec2 bottomrightworld = {bottomright.x + docTransform.position.x,
                           bottomright.y + docTransform.position.y};
  return worldToScreen(bottomrightworld);
}
bool Viewport::isPointInCanvas(vec2 screenPos,
                               const Transform2D &docTransform) const {
  vec2 canvasPos = screenToCanvas(screenPos, docTransform);
  return canvasPos.x >= 0.0f && canvasPos.x < m_canvasWidth &&
         canvasPos.y >= 0.0f && canvasPos.y < m_canvasHeight;
}
void Viewport::setScreenRect(SDL_FRect rect) { this->m_screenRect = rect; }
SDL_FRect Viewport::getScreenRect() const { return m_screenRect; }

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
  LX_ASSERT(factor > 0.0f, "Invalid zoom factor");
  float oldZoom = m_zoom;
  m_zoom *= factor;
  m_zoom = std::clamp(m_zoom, MIN_ZOOM, MAX_ZOOM);

  vec2 worldBefore = {(screenPoint.x - m_pan.x) / oldZoom,
                      (screenPoint.y - m_pan.y) / oldZoom};

  m_pan.x = screenPoint.x - worldBefore.x * m_zoom;
  m_pan.y = screenPoint.y - worldBefore.y * m_zoom;
  clampPan();
}

void Viewport::onCanvasResized(int cW, int cH) {
  LX_ASSERT(cW > 0 && cH > 0, "Invalid canvas dimensions");
  m_canvasWidth = cW;
  m_canvasHeight = cH;

  clampPan();
  Logger::debug(std::format("ViewPort Notified: canvas now {}x{}", cW, cH));
}
void Viewport::clampPan() {
  if (m_canvasWidth <= 0 || m_canvasHeight <= 0) {
    return;
  }

  float screenW = m_screenRect.w;
  float screenH = m_screenRect.h;
  float canvasScreenW = m_canvasWidth * m_zoom;
  float canvasScreenH = m_canvasHeight * m_zoom;

  float maxPanX = screenW;
  float minPanX = screenW - canvasScreenW;
  float maxPanY = screenH;
  float minPanY = screenH - canvasScreenH;

  m_pan.x = std::clamp(m_pan.x, minPanX, maxPanX);
  m_pan.y = std::clamp(m_pan.y, minPanY, maxPanY);
}

void Viewport::fitCanvasToScreen() {
  if (m_canvasWidth <= 0 || m_canvasHeight <= 0) {
    return;
  }

  float screenW = m_screenRect.w;
  float screenH = m_screenRect.h;
  float zoomX = screenW / m_canvasWidth;
  float zoomY = screenH / m_canvasHeight;
  float fitZoom = std::min(zoomX, zoomY) * MARGIN;

  setZoom(fitZoom);

  float canvasScreenW = m_canvasWidth * m_zoom;
  float canvasScreenH = m_canvasHeight * m_zoom;

  m_pan.x = (screenW - canvasScreenW) * 0.5f;
  m_pan.y = (screenH - canvasScreenH) * 0.5f;
  Logger::debug(std::format("Fit canvas to screen: zoom={}", m_zoom));
}
