
#pragma once
#include "ClickTool.h"
#include "Editor/Commands/SnapshotCommand.h"

class FloodFill : public ClickTool {
  uint32_t m_color = 0;
  std::unique_ptr<SnapshotCommand> m_command;
  vec2 pos;

public:
  std::string ToolID = "floodfill";
  std::unique_ptr<Command> onMouseClick(vec2 pos, ToolContext &ctx) override;
};
