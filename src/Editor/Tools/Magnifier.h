#pragma once
#include "ClickTool.h"

class Magnifier : public ClickTool {
public:
  std::string ToolID = "magnifier";
  std::unique_ptr<Command> onMouseClick(vec2 pos, ToolContext &ctx) override;

  // Set by Editor immediately before dispatch, same convention as Eyedropper.
  void setButton(bool isRightClick) { m_rightClick = isRightClick; }

private:
  // NOTE: the existing toolbar options box (Toolbar::renderZoomLevels) shows
  // presets 1/2/6/8, but the tool spec explicitly requires 1/2/4/8. These two
  // disagree in the current codebase. This tool follows the spec (1/2/4/8).
  // Toolbar::renderZoomLevels and ToolSettings::zoomLevel should be updated
  // to match if the toolbar buttons are meant to drive this tool — currently
  // they're separate UI elements that aren't wired together.
  static constexpr int kLevels[] = {1, 2, 4, 8};
  static constexpr int kLevelCount = 4;

  int currentLevelIndex(float zoom) const;
  bool m_rightClick = false;
};
