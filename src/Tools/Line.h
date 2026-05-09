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
  void onMouseDown(vec2 pos, SDL_Surface *surface) override;
  void onMouseMove(vec2 pos, SDL_Surface *surface, PreviewSystem *ps) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, SDL_Surface *surface,
                                     PreviewSystem *ps) override;
};
