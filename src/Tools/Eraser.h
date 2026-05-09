#pragma once

#include "./BaseTool.h"

class Eraser : public BaseTool {
  bool drawing = false;
  bool useXOR = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2 Start;
  // vec2 lastpos;
  uint32_t color = COLORS::WHITE;

public:
  int brushSize = 1;
  void onMouseDown(vec2 pos, SDL_Surface *surface) override;
  void onMouseMove(vec2 pos, SDL_Surface *surface, PreviewSystem *ps) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, SDL_Surface *surface,
                                     PreviewSystem *ps) override;
};
