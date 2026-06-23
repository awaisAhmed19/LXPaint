#pragma once

#include "BaseTool.h"
#include "Editor/Commands/SnapshotCommand.h"
#include <vector>

/*
  Polygon — Microsoft-Paint-style multi-click polygon.

  Interaction model:
    click (down+up) -> places a vertex (first click starts the polygon).
                        If this click landed within kCloseRadius of the
                        first vertex AND at least kMinVertices vertices
                        already exist, it CLOSES the polygon instead:
                        draws the final edge, fills if required, commits
                        one SnapshotCommand, and resets to idle.
    mouse move       -> updates a rubber-band edge from the last committed
                        vertex to the cursor. Must fire even when no mouse
                        button is held — see wantsHoverMoves() below and the
                        corresponding Editor::handleEvent change that
                        forwards hover-only moves to tools that opt in.
    Escape           -> cancels, clears preview, no history entry.
    right click /
    double click     -> no action (per spec; Editor does not route these
                        to Polygon specially, so nothing to do here, but
                        documented for clarity).

  Like every existing BaseTool, the actual undo-able commit happens in
  onMouseUp (Editor::handleEvent pushes whatever onMouseUp returns). A
  "click" reaching this tool is therefore always a down+up pair with ~zero
  drag distance — onMouseDown decides what the click *means* (new vertex vs.
  closing click) and onMouseUp performs the corresponding canvas mutation
  and returns the SnapshotCommand only when a close just happened.
*/

class Polygon : public BaseTool {
public:
  enum class FillMode {
    Outline, // edges only
    Opaque,  // edges + interior filled with background color on close
    Filled,  // edges + interior filled with foreground color on close
  };

private:
  enum class State { Idle, Drawing };

  State m_state = State::Idle;
  bool m_pendingClose = false; // set by onMouseDown, consumed by onMouseUp

  std::vector<vec2> m_points; // committed vertices (does not include the
                              // live rubber-band point under the cursor)
  vec2 m_cursor{0.f, 0.f};    // last known mouse position, for the
                              // rubber-band edge and the closing hitbox

  SDL_Rect m_bounds{0, 0, 0, 0}; // running bounds of m_points ∪ m_cursor
  std::unique_ptr<SnapshotCommand> m_command;

  // Classic MS Paint's closing hitbox is tight — a handful of pixels, not
  // a generous catch radius. 50px (the original spec number) was copied
  // verbatim without sanity-checking it against real Paint behavior; at
  // that size, polygons would close accidentally on almost any click near
  // the start vertex. 8px matches actual Paint feel.
  //
  // NOTE: pos is canvas-space here, not screen-space. At high zoom, 8
  // canvas-px is large on screen (easy/over-eager to hit); at low zoom
  // it's small (hard to hit). A zoom-aware radius (8px / viewport.getZoom())
  // would be more correct but is a separate change from the requested
  // 50->8 fix, so left as-is — flagging it rather than quietly bundling it.
  static constexpr float kCloseRadius = 8.0f;
  static constexpr size_t kMinVertices = 3;

  void expandBounds(vec2 p);
  SDL_Rect computeFullBounds(int padding, int maxW, int maxH) const;
  void redrawPreview(ToolContext &ctx);
  bool nearStart(vec2 pos) const;
  FillMode currentFillMode(ToolContext &ctx) const;
  void reset();

public:
  bool usesPreview() const override { return true; }

  // Polygon needs onMouseMove called continuously between clicks (no button
  // held) to draw the rubber-band edge — see Editor::handleEvent change.
  bool wantsHoverMoves() const override { return m_state == State::Drawing; }

  void onMouseDown(vec2 pos, ToolContext &ctx) override;
  void onMouseMove(vec2 pos, ToolContext &ctx) override;
  std::unique_ptr<Command> onMouseUp(vec2 pos, ToolContext &ctx) override;

  bool onKeyDown(SDL_Scancode scancode, ToolContext &ctx) override;

  void deactivate() override;
};
