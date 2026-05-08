#pragma once
#include "../Document/Canvas.h"
#include "../Globals.h"
#include <algorithm>
namespace Renderer {

void drawCircle(vec2 center, float radius, SDL_Surface *canvas, uint32_t color);
void floodFill(vec2 start, SDL_Surface *canvas, uint32_t newColor);

void dda(vec2 start, vec2 end, SDL_Surface *canvas, uint32_t color,
         int brushSize, bool useXOR);

void bresenham(vec2 start, vec2 end, SDL_Surface *canvas, uint32_t color,
               int brushSize, bool useXOR);
} // namespace Renderer
