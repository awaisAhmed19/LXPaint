#include "EditorDocument.h"

EditorDocument::EditorDocument(int width, int height)
    : m_canvas(width, height), m_preview(width, height) {}

Canvas &EditorDocument::getCanvas() { return m_canvas; }
const Canvas &EditorDocument::getCanvas() const { return m_canvas; }

PreviewLayer &EditorDocument::getPreview() { return m_preview; }
const PreviewLayer &EditorDocument::getPreview() const { return m_preview; }

void EditorDocument::resize(int width, int height, const ResizePolicy &policy) {
  m_canvas.resize(width, height, policy);
  m_preview.resize(width, height, policy);
  // TODO:
  // m_preview.resize(...)
  // selection.resize(...)
  // layer stack resize(...)
  //
}
