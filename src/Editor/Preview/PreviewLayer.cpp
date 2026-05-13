#include "PreviewLayer.h"

PreviewLayer::PreviewLayer(int width, int height)
    : RenderTarget(width, height) {

  clearRGBA(255, 255, 255, 255);
}
