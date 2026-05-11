#pragma once
#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include "./BaseTool.h"

class Eraser : public BaseTool {
  bool m_useXOR = false;
  vec2 m_start;
  vec2 m_last;
  uint32_t m_color = COLORS::WHITE;

public:
  int brushSize = 1;
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
