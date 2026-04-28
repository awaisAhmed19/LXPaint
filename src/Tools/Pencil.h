#pragma once
#include "./BaseTool.h"
class Pencil : public BaseTool {
  bool drawing = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2<float> startpos;
  vec2<float> lastpos;

public:
  // ~Pencil() {
  //   if (currentSnapshot)
  //     SDL_DestroySurface(currentSnapshot);
  // }
  const int brushSize = 1;
  uint32_t color = COLORS::BLACK;

  void onMouseDown(vec2<float> pos, Canvas &canvas) override;
  void onMouseMove(vec2<float> pos, Canvas &canvas) override;
  Command *onMouseUp(vec2<float> pos, Canvas &canvas) override;
  // void bresenham(vec2<float> start, vec2<float> end, Canvas &canvas);
  // void dda(vec2<float> start, vec2<float> end, Canvas &canvas);
};
