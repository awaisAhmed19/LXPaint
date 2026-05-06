#pragma once
#include "../Globals.h"
#include "Canvas.h"
#include <algorithm>
namespace Renderer {

// Future expansion is now easy!
void drawCircle(vec2 center, float radius, Canvas &canvas, uint32_t color);
void floodFill(vec2 start, Canvas &canvas, uint32_t newColor);

void dda(vec2 start, vec2 end, Canvas &canvas, uint32_t color, int brushSize,
         bool useXOR);

void bresenham(vec2 start, vec2 end, Canvas &canvas, uint32_t color,
               int brushSize, bool useXOR);
} // namespace Renderer
