#pragma once
#include "../App/Globals.h"
#include "../Editor/Viewport/Viewport.h"
#include "./RenderTarget.h"
#include "./Transform2D.h"
#include <SDL3/SDL.h>
class Renderer {
protected:
  SDL_Renderer *m_renderer;

public:
  Renderer(SDL_Renderer *renderer);
  void begin();
  void renderTarget(RenderTarget &target, const Viewport &viewport,
                    const Transform2D &transform);
  void end();
  SDL_Renderer *getSDLRenderer() const;

private:
  void sync(RenderTarget &target);
  void ensureTexture(RenderTarget &target);
};
