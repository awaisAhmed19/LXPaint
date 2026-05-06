#pragma once
#include "./BaseTool.h"

class Line : public BaseTool {
  bool drawing = false;
  bool useXOR = false;
  int minX, minY, maxX, maxY, maxW, maxH;
  vec2 Start, Last;
  SDL_Rect Boundbox{}, prevBound{};

public:
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  void onMouseDown(vec2 pos, Canvas &canvas) override;
  void onMouseMove(vec2 pos, Canvas &canvas) override;
  Command *onMouseUp(vec2 pos, Canvas &canvas) override;
};
