#pragma once
#include <SDL3/SDL.h>

#include <vector>

#include "PreviewLayer.h"
#include "RenderTarget.h"

enum class ResizeAnchor { TOPLEFT, CENTER };
enum class ResizeFill { TRANSPARENT, BACKGROUNDCOLOR };

struct ResizePolicy {
  ResizeAnchor anchor = ResizeAnchor::TOPLEFT;
  ResizeFill fill = ResizeFill::TRANSPARENT;
  bool preservePixels = true;
};

struct ResizeRequest {
  int newWidth, newHeight;
  ResizePolicy policy;
  bool preserveUndoHistory = true;
};

class Canvas : public RenderTarget {
private:
  std::vector<RenderTarget *> m_renderTargets;
  PreviewLayer m_preview;

public:
  Canvas(int width, int height);
  void resize(int w, int h, ResizePolicy &resizePolicy);
  SDL_Rect computeSourceRect(int oldW, int oldH, int newW, int newH,
                             const ResizePolicy &policy) const;
  SDL_Rect computeDestinationRect(int oldW, int oldH, int newW, int newH,
                                  const ResizePolicy &policy) const;
  bool validateResize(const ResizeRequest &req);
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
