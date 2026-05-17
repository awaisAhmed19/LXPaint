#pragma once
#include "../Commands/SnapshotCommand.h"
#include "../Rendering/Rasterizer.h"
#include "../Systems/Logger.h"
#include "../Systems/Profiler.h"
#include "./StrokeTool.h"

class Eraser : public StrokeTool {
  bool m_useXOR = false;
  uint32_t m_color = COLORS::WHITE;
  std::unique_ptr<SnapshotCommand> m_command;
  vec2 m_start;
  vec2 m_last;

public:
  int brushSize = 3;
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
