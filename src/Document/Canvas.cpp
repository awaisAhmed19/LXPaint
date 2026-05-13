#include "./Canvas.h"
Canvas::Canvas(int w, int h) : RenderTarget(w, h) {
  clearRGBA(255, 255, 255, 255);
}
