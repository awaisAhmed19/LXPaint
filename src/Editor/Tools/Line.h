#pragma once
#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include "./BaseTool.h"
class Line : public BaseTool {
private:
  vec2 m_start;
  vec2 m_last;

public:
  int brushSize = 1;
  uint32_t color = COLORS::BLACK;

  void onMouseDown(vec2 pos, ToolContext &ctx) override;

  void onMouseMove(vec2 pos, ToolContext &ctx) override;

  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
