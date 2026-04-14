#pragma once
#include "../Globals.h"
#include "../core/Canvas.h"
#include "./BaseTool.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <vector>

class Pencil : public BaseTool {
  bool drawing = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2 startpos;
  vec2 lastpos;

public:
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;

  // static void _putpixel(SDL_Surface* surface, vec2 pos, uint32_t color) ;
  void onMouseDown(vec2 pos, Canvas &canvas) override;
  void onMouseMove(vec2 pos, Canvas &canvas) override;
  Command *onMouseUp(vec2 pos, Canvas &canvas) override;
  void bresenham(vec2 start, vec2 end, Canvas &canvas, uint32_t color,
                 int brushSize);

  void dda(vec2 start, vec2 end, Canvas &canvas, uint32_t color);
};
