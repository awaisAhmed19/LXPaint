#pragma once
#include <SDL3/SDL.h>

#include <vector>

#include "RenderTarget.h"
class Canvas : public RenderTarget {
private:
  // std::vector<RenderTarget *> m_renderTargets;
public:
  Canvas(int w, int h);

  void resize(int w, int h, const ResizePolicy &policy);
  /* resize Transform pipeline
    {
      1. allocate new surface
      2. clear with fill policy
      3. compute copy offset
      4. copy old pixels
      5. swap surfaces
      6. invalidate textures
      7. resize preview /layers
      8. notify viewport
    }
    */
};
