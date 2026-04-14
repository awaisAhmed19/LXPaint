#pragma once
#include "../Globals.h"
#include "../core/Canvas.h"
#include "../core/Profiler.h"
#include "./BaseTool.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <vector>
class Pencil : public BaseTool {
  bool drawing = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2<float> startpos;
  vec2<float> lastpos;

public:
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  void onMouseDown(vec2<float> pos, Canvas &canvas) override;
  void onMouseMove(vec2<float> pos, Canvas &canvas) override;
  Command *onMouseUp(vec2<float> pos, Canvas &canvas) override;
  void bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
                 uint32_t color, int brushSize);

  void dda(vec2<float> start, vec2<float> end, Canvas &canvas, uint32_t color,
           int brushSize);
};
