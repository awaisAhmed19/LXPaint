#pragma once
#include "../Rendering/RenderTarget.h"
#include <SDL3/SDL.h>

class Canvas : public RenderTarget {
public:
  Canvas(int width, int height);
};
