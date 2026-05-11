#pragma once

#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include "./BaseTool.h"

class Rect : public BaseTool {
private:
  bool m_useXOR = false;
  vec2 m_start;
  SDL_Rect m_prevBounds{};

  enum class RectMode { STROKE, FILL, WHITEFILL };

public:
  uint32_t Wcolor = COLORS::WHITE;
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
