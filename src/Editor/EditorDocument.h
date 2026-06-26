#pragma once

#include "Document/Canvas.h"
#include "Document/PreviewLayer.h"
#include <filesystem>
class EditorDocument {
public:
  EditorDocument(int width, int height);

  Canvas &getCanvas();
  const Canvas &getCanvas() const;

  PreviewLayer &getPreview();
  const PreviewLayer &getPreview() const;

  void resize(int width, int height, const ResizePolicy &policy);

  // ── Document-level image operations ──────────────────────────────────
  // These mutate the Canvas pixel data directly. Preview is unaffected by
  // all of these except the rotations, which must stay the same size as
  // the canvas (Renderer assumes preview/canvas share dimensions — see
  // Editor::render, which draws both through the same Viewport/Transform).
  void invertColors();
  void flipHorizontal();
  void flipVertical();
  void rotate90CW();
  void rotate90CCW();

  // Fills the canvas with the given color (used for "Clear Image").
  void clear(uint32_t color);

  bool isModified() const;
  void markModified();
  void markSaved();

  bool hasPath() const;
  const std::filesystem::path &path() const;
  void setPath(const std::filesystem::path &);

private:
  bool m_modified = false;
  Canvas m_canvas;
  PreviewLayer m_preview;
  std::filesystem::path m_path;
};
