#pragma once
#include "App/Globals.h"
#include "SelectionTool.h"
class RectSelection : public SelectionTool {
private:
  vec2 m_start{};
  vec2 m_current{};
  vec2 m_dragLast{};

  bool containsPoint(vec2 pos) const;
  void buildMask();
  void redrawPreview(ToolContext &ctx);

public:
  bool usesPreview() const override { return true; }

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
  void deactivate() override;
};
