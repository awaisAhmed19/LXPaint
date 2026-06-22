#pragma once
#include "ClickTool.h"

class Eyedropper : public ClickTool {
public:
  std::string ToolID = "eyedropper";
  std::unique_ptr<Command> onMouseClick(vec2 pos, ToolContext &ctx) override;

  // Left vs right click must be distinguishable; ClickTool::onMouseClick
  // only gives position, so Editor passes the button via this setter
  // immediately before calling onMouseClick (see Editor::handleEvent).
  void setButton(bool isRightClick) { m_rightClick = isRightClick; }

private:
  bool m_rightClick = false;
};
