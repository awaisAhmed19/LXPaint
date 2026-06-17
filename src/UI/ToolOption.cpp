#include "UI/ToolOption.h"
namespace UI {

void ToolOptions::render(Editor &editor) {
  switch (editor.getActiveTool()) {
  case ToolType::Pencil:
    renderPencil(editor);
    break;

  case ToolType::Brush:
    renderBrush(editor);
    break;

  case ToolType::Line:
    renderLine(editor);
    break;

  case ToolType::Rectangle:
    renderRectangle(editor);
    break;

  case ToolType::Ellipse:
    renderEllipse(editor);
    break;

  default:
    break;
  }
}
}; // namespace UI
