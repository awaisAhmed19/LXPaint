#pragma once

#include "./BaseTool.h"

class Eraser : public BaseTool {
  bool drawing = false;
  bool useXOR = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2<float> Start;
  // vec2<float> lastpos;
  uint32_t color = COLORS::WHITE;

public:
  int brushSize = 1;
  void onMouseDown(vec2<float> pos, Canvas &canvas) override;
  void onMouseMove(vec2<float> pos, Canvas &canvas) override;
  Command *onMouseUp(vec2<float> pos, Canvas &canvas) override;
};
