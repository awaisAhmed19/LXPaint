#include "PreviewLayer.h"

PreviewLayer::PreviewLayer(int width, int height)
    : RenderTarget(width, height) {

  clearRGBA(0, 0, 0, 0);
}
