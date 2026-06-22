#pragma once
#include "Editor/Commands/SnapshotCommand.h"
#include "Rendering/Rasterizer.h"
#include "StrokeTool.h"
#include <memory>

class Brush : public StrokeTool {
private:
  vec2 m_last = {0.0f, 0.0f};
  vec2 m_start = {0.0f, 0.0f};
  std::unique_ptr<SnapshotCommand> m_command;

public:
  // Shape and size come from ToolSettings (brushShape reused exactly as
  // the existing ToolSettings::BrushShape enum, and strokeWidth carrying
  // the 1/2/4 size — same fields the Toolbar options box already edits via
  // Toolbar::renderBrushShapes).
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;

private:
  static Rasterizer::BrushShape toRasterShape(int toolSettingsShape);
  static int clampSize(float size);
};
