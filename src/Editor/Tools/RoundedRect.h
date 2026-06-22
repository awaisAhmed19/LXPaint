#pragma once

#include "Editor/Commands/SnapshotCommand.h"
#include "GeometricTool.h"

class RoundedRect : public GeometricTool {
private:
  bool m_useXOR = false;

  vec2 m_start;
  vec2 m_last;

  SDL_Rect m_affected{};
  std::unique_ptr<SnapshotCommand> m_command;

  enum class RectMode {
    STROKE,
    FILL,
    WHITEFILL,
  };

  int m_cornerRadius = 16;

public:
  uint32_t Wcolor = COLORS::WHITE;
  uint32_t color = COLORS::BLACK;
  const int brushSize = 1;

  bool usesPreview() const override { return true; }

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
};
