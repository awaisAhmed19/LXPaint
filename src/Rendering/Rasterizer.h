#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>

#include "App/Globals.h"
namespace Rasterizer {
void bresenham(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
               int brushSize, bool useXOR);

void dda(vec2 start, vec2 end, SDL_Surface *surface, uint32_t color,
         int brushSize, bool useXOR);
void floodFill(SDL_Surface *surface, vec2 pos, uint32_t color,
               uint32_t newcolor);
void drawEllipse_theta(SDL_Surface *surface, int x, int y, int w, int h,
                       uint32_t color);
void drawEllipse(SDL_Surface *surface, int xc, int yc, int rx, int ry,
                 uint32_t color);
void drawCircle(SDL_Surface *surface, int x_centre, int y_centre, int r,
                uint32_t color);
void drawPixel(SDL_Surface *surface, int x, int y, uint32_t color);

void drawVerticalSpan(SDL_Surface *surface, int x, int y, int thickness,
                      uint32_t color, bool useXOR);

void drawHorizontalSpan(SDL_Surface *surface, int x, int y, int thickness,
                        uint32_t color, bool useXOR);

void rectFill(SDL_Surface *surface, int minX, int minY, int maxX, int maxY,
              uint32_t color);

void rectFillWhite(SDL_Surface *surface, int minX, int minY, int maxX,
                   int maxY);

void drawRectStroke(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                    int brushSize);

void drawRectFill(SDL_Surface *surface, vec2 a, vec2 b, uint32_t color,
                  int brushSize, bool isWhiteFill);

void floodFill(SDL_Surface *surface, vec2 pos, uint32_t newcolor);

}; // namespace Rasterizer
