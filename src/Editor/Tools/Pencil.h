#pragma once
#include "../Commands/SnapshotCommand.h"
#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include "./BaseTool.h"
#include <chrono>
#include <memory>

class Pencil : public BaseTool {
private:
  vec2 m_last = {0.0f, 0.0f};
  vec2 m_start = {0.0f, 0.0f};
  std::unique_ptr<SnapshotCommand> m_command;

public:
  const int brushSize = 3;

  uint32_t color = COLORS::BLACK;

  void onMouseDown(vec2 pos, ToolContext &ctx) override;

  void onMouseMove(vec2 pos, ToolContext &ctx) override;

  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
