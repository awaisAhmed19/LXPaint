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
  markModified();
  // TODO:
  // selection.resize(...)
  // layer stack resize(...)
  //
}

bool EditorDocument::isModified() const { return m_modified; }
void EditorDocument::markModified() { m_modified = true; }
void EditorDocument::markSaved() { m_modified = false; }
bool EditorDocument::hasPath() const { return !m_path.empty(); }
const std::filesystem::path &EditorDocument::path() const { return m_path; }
void EditorDocument::setPath(const std::filesystem::path &path) {
  m_path = path;
}

void EditorDocument::invertColors() {
  m_canvas.invertColors();
  m_canvas.invalidateRect({0, 0, m_canvas.getWidth(), m_canvas.getHeight()});
  markModified();
}

void EditorDocument::flipHorizontal() {
  m_canvas.flipHorizontal();
  m_canvas.invalidateRect({0, 0, m_canvas.getWidth(), m_canvas.getHeight()});
  markModified();
}

void EditorDocument::flipVertical() {
  m_canvas.flipVertical();
  m_canvas.invalidateRect({0, 0, m_canvas.getWidth(), m_canvas.getHeight()});
  markModified();
}

void EditorDocument::rotate90CW() {
  m_canvas.rotate90CW();
  // Canvas dimensions just swapped (w<->h) — preview must follow so the
  // two RenderTargets stay the same size, matching the invariant
  // Renderer/Editor rely on (see Editor::render rendering both through
  // the same Viewport/Transform2D).
  ResizePolicy policy;
  policy.anchor = ResizeAnchor::TOPLEFT;
  policy.fill = ResizeFill::TRANSPARENT;
  m_preview.allocate(m_canvas.getWidth(), m_canvas.getHeight(),
                     RenderTarget::FillColor::TRANSPARENT);
  m_canvas.invalidateRect({0, 0, m_canvas.getWidth(), m_canvas.getHeight()});
  markModified();
}

void EditorDocument::rotate90CCW() {
  m_canvas.rotate90CCW();
  m_preview.allocate(m_canvas.getWidth(), m_canvas.getHeight(),
                     RenderTarget::FillColor::TRANSPARENT);
  m_canvas.invalidateRect({0, 0, m_canvas.getWidth(), m_canvas.getHeight()});
  markModified();
}
void EditorDocument::clear(uint32_t color) {
  m_canvas.clear(color);
  markModified();
}
