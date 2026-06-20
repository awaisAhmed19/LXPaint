#pragma once
#include "Editor/Commands/SnapshotCommand.h"
#include "StrokeTool.h"
#include <memory>

class AirBrush : public StrokeTool {
private:
  vec2 m_last = {0.0f, 0.0f};
  vec2 m_start = {0.0f, 0.0f};
  std::unique_ptr<SnapshotCommand> m_command;

public:
  const int brushSize = 1;

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
