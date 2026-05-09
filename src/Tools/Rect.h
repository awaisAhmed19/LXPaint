#pragma once
#include "./BaseTool.h"

class Rect : public BaseTool {
  bool drawing = false;
  bool useXOR = false;
  int minX, minY, maxX, maxY, maxW, maxH;
  vec2 Start, Last;
  SDL_Rect Boundbox{}, prevBound{};
  enum class rect_type { STROKE, FILL, WHITEFILL };

public:
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  // void Rect_fill(vec2 start, int w, int h);
  void onMouseDown(vec2 pos, SDL_Surface *surface) override;
  void onMouseMove(vec2 pos, SDL_Surface *surface, PreviewSystem *ps) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, SDL_Surface *surface,
                                     PreviewSystem *ps) override;
};
