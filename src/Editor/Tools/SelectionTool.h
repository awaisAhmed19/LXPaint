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

protected:
  SelectionState m_state = SelectionState::Idle;

  Selection m_selection;
  void copyFromCanvas(SDL_Surface *canvas);
  void clearSelection();
  std::unique_ptr<Command> commitSelection(ToolContext &ctx);
  void drawSelectionOutline(ToolContext &ctx);
};
