#pragma once
#include "App/Globals.h"
#include <SDL3/SDL.h>
namespace Rasterizer {
void bresenham(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
               int brushSize, bool useXOR);

void dda(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
         int brushSize, bool useXOR);

void drawPixel(SDL_Surface *surface, int x, int y, uint32_t color);

void drawVerticalSpan(SDL_Surface *surface, int x, int y, int thickness,
                      uint32_t color, bool useXOR);

void drawHorizontalSpan(SDL_Surface *surface, int x, int y, int thickness,
                        uint32_t color, bool useXOR);

void rectFill(SDL_Surface *surface, int minX, int minY, int maxX, int maxY,
              uint32_t color);

void rectFillWhite(SDL_Surface *surface, int minX, int minY, int maxX,
                   int maxY);

void drawRect(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
              int brushSize);
}; // namespace Rasterizer
