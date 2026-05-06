#pragma once
#include "./BaseTool.h"
class Pencil : public BaseTool {
  bool drawing = false;
  bool useXOR = false;
  SDL_Surface *currentSnapshot = nullptr;
  int minX, minY, maxX, maxY, maxW, maxH;
  vec2 Start, Last;
  SDL_Rect Boundbox{};
  DrawCommand *activeCommand = nullptr;

public:
  const int brushSize = 3;
  uint32_t color = COLORS::BLACK;

  void onMouseDown(vec2 pos, Canvas &canvas) override;
  void onMouseMove(vec2 pos, Canvas &canvas) override;
  Command *onMouseUp(vec2 pos, Canvas &canvas) override;
};
