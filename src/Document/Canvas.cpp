#include "Canvas.h"

#include "Systems/Logger.h"

Canvas::Canvas(int w, int h) : RenderTarget(w, h) {
  clearRGBA(255, 255, 255, 255);
}

void Canvas::resize(int w, int h, const ResizePolicy &policy) {
  RenderTarget::resize(w, h, policy);
}
