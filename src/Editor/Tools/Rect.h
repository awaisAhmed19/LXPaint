#pragma once

#include "Editor/Commands/SnapshotCommand.h"
#include "GeometricTool.h"

class Rect : public GeometricTool {
private:
  vec2 m_start;
  vec2 m_last;
  std::unique_ptr<SnapshotCommand> m_command;
  SDL_Rect m_affected{};
  enum class FillMode {
    Outline, // edges only
    Opaque,  // edges + interior filled with background color on close
    Filled,  // edges + interior filled with foreground color on close
  };
  FillMode currentFillMode(ToolContext &ctx) const;

public:
  uint32_t Wcolor = COLORS::WHITE;
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;
  bool usesPreview() const override { return true; }
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
