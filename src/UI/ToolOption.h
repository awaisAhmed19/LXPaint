#pragma once

#include "Editor/Editor.h"

namespace UI {

class ToolOptions {
public:
  ToolOptions() = default;

  void render(Editor &editor);

private:
  void renderPencil(Editor &editor);
  void renderBrush(Editor &editor);
  void renderLine(Editor &editor);
  void renderRectangle(Editor &editor);
  void renderEllipse(Editor &editor);
};

} // namespace UI
