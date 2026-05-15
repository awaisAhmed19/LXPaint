#pragma once
#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"

#include "../Commands/DrawStrokeCommand.h"
#include "./BaseTool.h"
#include <chrono>
#include <memory>

class Pencil : public BaseTool {
private:
  vec2 m_last;
  std::unique_ptr<DrawStrokeCommand> m_activeCommand;

public:
  const int brushSize = 3;

  uint32_t color = COLORS::BLACK;

  void onMouseDown(vec2 pos, ToolContext &ctx) override;

  void onMouseMove(vec2 pos, ToolContext &ctx) override;

  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
