
#pragma once
#include "../../App/Globals.h"
#include <SDL3/SDL.h>

class Viewport {
private:
  vec2 m_pan = {0.0f, 0.0f};
  float m_zoom = 1.0f;
  SDL_FRect m_screenRect;

public:
  // Viewport();
  // ~Viewport();
  vec2 screenToWorld(vec2 screen) const;

  vec2 worldToScreen(vec2 world) const;

  SDL_FRect worldRectToScreen(SDL_FRect rect) const;

  vec2 getPan() const;
  float getZoom() const;
  void setPan(vec2 pan);
  void setZoom(float zoom);

  bool isVisible(SDL_FRect rect) const;
  SDL_FRect getVisibleCanvasBounds() const;
  void setScreenRect(SDL_FRect rect);
  void ZoomAt(vec2 screenPoint, float factor);
};
