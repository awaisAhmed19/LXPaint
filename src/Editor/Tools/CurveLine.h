#pragma once

#include "Editor/Commands/SnapshotCommand.h"
#include "GeometricTool.h"

/*
  Cubic Bezier interaction — three click-drag cycles:

    Cycle 1:  drag  P0 → P3   (endpoints)
    Cycle 2:  drag  P1         (control point 1)
    Cycle 3:  drag  P2 → commit (control point 2)
*/
enum class BezierPhase {
  IDLE,      // Ready for the first click
  ENDPOINTS, // Cycle 1 in progress: dragging P0 → P3
  CP1,       // Cycle 2 in progress: dragging control point P1
  CP2,       // Cycle 3 in progress: dragging control point P2 → commit
};

class CurveLine : public GeometricTool {
private:
  BezierPhase m_phase = BezierPhase::IDLE;

  vec2 m_p0{}; // Anchor: start point
  vec2 m_p1{}; // Control point 1
  vec2 m_p2{}; // Control point 2
  vec2 m_p3{}; // Anchor: end point

  SDL_Rect m_affected{};
  std::unique_ptr<SnapshotCommand> m_command;

  // ── Preview internals ──────────────────────────────────
  void updatePreview(ToolContext &ctx, int lineWidth);

  void drawBezierToSurface(SDL_Surface *surf, uint32_t color,
                           int lineWidth) const;

  void drawDashedLine(SDL_Surface *surf, vec2 a, vec2 b, uint32_t color) const;

  void drawSquareMarker(SDL_Surface *surf, vec2 pos, uint32_t fill,
                        uint32_t border) const;

  SDL_Rect computeAffectedRect(int lineWidth, int surfW, int surfH) const;

  // Place P1 and P2 at the 1/3 and 2/3 points along P0→P3
  void setDefaultControlPoints();

public:
  bool usesPreview() const override { return true; }

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;

  void deactivate() override;
};
