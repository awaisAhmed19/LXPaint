#pragma once
#include "Editor/Commands/SnapshotCommand.h"
#include "GeometricTool.h"
class Line : public GeometricTool {
private:
  vec2 m_start;
  vec2 m_last;
  std::unique_ptr<SnapshotCommand> m_command;

public:
  int brushSize = 1;
  uint32_t color = COLORS::BLACK;
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  bool usesPreview() const override { return true; }
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
