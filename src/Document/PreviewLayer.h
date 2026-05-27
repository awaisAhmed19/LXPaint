#pragma once
#include <SDL3/SDL.h>

#include "RenderTarget.h"

class PreviewLayer : public RenderTarget {
public:
  PreviewLayer(int width, int height);
};
