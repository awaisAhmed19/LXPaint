#pragma once

#include "BaseTool.h"
#include "Editor/Interaction/ToolContext.h"
#include <vector>
enum class SelectionState { Idle, Drawing, Selected, Dragging };
struct Selection {
  SDL_Rect bounds;
  std::vector<uint8_t> mask;
  SDL_Surface *pixels = nullptr;
  vec2 offset{0.f, 0.f};
  bool empty() const { return mask.empty(); }
};

class SelectionTool : public BaseTool {
public:
  virtual ~SelectionTool();
  // Promote to public — Editor needs to clear the active selection from
  // a menu action (Edit > Clear Selection), not just internally on
  // deactivate/click-outside as today.
  void clearSelection();

  // New: select the entire canvas. Builds a full-canvas mask exactly the
  // way RectSelection::buildMask already does for a drawn rect, just
  // pre-sized to (0,0,w,h) instead of a user drag.
  void selectAllCanvas(ToolContext &ctx);

protected:
  SelectionState m_state = SelectionState::Idle;

  Selection m_selection;
  void copyFromCanvas(SDL_Surface *canvas);
  std::unique_ptr<Command> commitSelection(ToolContext &ctx);
  void drawSelectionOutline(ToolContext &ctx);
};
