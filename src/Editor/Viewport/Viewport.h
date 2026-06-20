
#pragma once
#include <SDL3/SDL.h>

#include "App/Globals.h"

#include "Rendering/Transform2D.h"

class Viewport {
private:
  vec2 m_pan = {0.0f, 0.0f};
  float m_zoom = 1.0f;
  SDL_FRect m_screenRect;
  int m_canvasWidth = 0;
  int m_canvasHeight = 0;

public:
  // Viewport();
  // ~Viewport();
  vec2 screenToWorld(vec2 screen) const;
  vec2 screenToCanvas(vec2 screen, const Transform2D &docTransform) const;
  vec2 worldToScreen(vec2 world) const;
  SDL_FRect worldRectToScreen(SDL_FRect rect) const;
  bool isCanvasLargerThanViewport() const;
  SDL_FRect getCanvasBoundsScreen() const;
  vec2 getCanvasTopLeftScreen(const Transform2D &docTransform) const;
  vec2 getCanvasTopRightScreen(const Transform2D &docTransform) const;
  vec2 getCanvasBottomLeftScreen(const Transform2D &docTransform) const;
  vec2 getCanvasBottomRightScreen(const Transform2D &docTransform) const;
  bool isPointInCanvas(vec2 screenPos, const Transform2D &docTransform) const;
  void setScreenRect(SDL_FRect rect);
  SDL_FRect getScreenRect() const;

  vec2 getPan() const;
  void setPan(vec2 pan);

  float getZoom() const;
  void setZoom(float zoom);

  bool isVisible(SDL_FRect rect) const;

  SDL_FRect getVisibleCanvasBounds() const;
  void ZoomAt(vec2 screenPoint, float factor);

  void onCanvasResized(int cW, int cH);
  void clampPan();

  void fitCanvasToScreen();
};
