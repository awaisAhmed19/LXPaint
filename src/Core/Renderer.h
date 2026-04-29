#pragma once
#include "../Globals.h"
#include "Canvas.h"
#include <algorithm>
namespace Renderer {

// Future expansion is now easy!
void drawCircle(vec2f center, float radius, Canvas &canvas, uint32_t color);
void floodFill(vec2f start, Canvas &canvas, uint32_t newColor);

void dda(vec2<float> start, vec2<float> end, Canvas &canvas, uint32_t color,
         int brushSize, bool useXOR);

void bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
               uint32_t color, int brushSize, bool useXOR);
} // namespace Renderer
