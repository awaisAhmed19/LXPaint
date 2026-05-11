#pragma once
#include "../App/Globals.h"
#include "./RenderTarget.h"
#include <SDL3/SDL.h>
class Renderer {
protected:
  SDL_Renderer *m_renderer;
  SDL_FRect m_viewport;
  float m_zoom = 1.0f;
  vec2 m_pan = {0, 0};

public:
  Renderer(SDL_Renderer *renderer);

  // void setCanvas(Canvas *canvas);
  // void setPreview(PreviewSystem *ps);

  void begin();
  void renderTarget(RenderTarget &target);
  void end();

  vec2 screenToCanvas(vec2 screen) const;
  vec2 canvasToScreen(vec2 canvas) const;

  void setPan(vec2 pan);
  void setZoom(float zoom);

private:
  void sync(RenderTarget &target);
};
