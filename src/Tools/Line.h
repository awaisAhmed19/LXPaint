#pragma once
#include "./BaseTool.h"

class Line : public BaseTool {
  bool drawing = false;
  SDL_Surface *currentSnapshot = nullptr;
  vec2<float> startpos;
  vec2<float> lastpos;
  vec2<float> lastMousePos; // Track for Dirty Rect
  enum class OptimizationMode { BRUTE_FORCE, DIRTY_RECT, OVERLAY };
  OptimizationMode g_OptMode = OptimizationMode::DIRTY_RECT;

public:
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  void onMouseDown(vec2<float> pos, Canvas &canvas) override;
  void onMouseMove(vec2<float> pos, Canvas &canvas) override;
  Command *onMouseUp(vec2<float> pos, Canvas &canvas) override;
  // void bresenham(vec2<float> start, vec2<float> end, Canvas &canvas,
  //                uint32_t color, int brushSize, bool useXOR);
  //
  // void dda(vec2<float> start, vec2<float> end, Canvas &canvas, uint32_t
  // color,
  //          int brushSize, bool useXOR);
};
