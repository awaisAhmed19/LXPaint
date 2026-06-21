#pragma once

#include "Editor/Interaction/ToolContext.h"
#include "SelectionTool.h"
#include <vector>

class Lasso : public SelectionTool {
private:
  std::vector<vec2> m_points;
  SDL_Rect m_bounds{0, 0, 0, 0};
  vec2 m_last{0.f, 0.f};
  vec2 m_start{0.f, 0.f};
  vec2 m_dragLast{0.f, 0.f};

  // SDL_Rect computeBounds() const;
  bool pointInPolygon(int x, int y) const;
  void buildMask();
  void expandBounds(vec2 p);
  bool containsPoint(vec2 pos) const;
  void redrawPreview(ToolContext &ctx);

public:
  bool usesPreview() const override { return true; }
  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;
  void deactivate() override;
};
