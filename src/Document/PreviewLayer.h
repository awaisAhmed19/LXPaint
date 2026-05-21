#pragma once
// #include "../Globals"
#include "RenderTarget.h"
#include <SDL3/SDL.h>
class PreviewLayer : public RenderTarget {
public:
  PreviewLayer(int width, int height);
};
