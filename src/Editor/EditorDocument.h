#pragma once

#include "Document/Canvas.h"
#include "Document/PreviewLayer.h"

class EditorDocument {
public:
  EditorDocument(int width, int height);

  Canvas &getCanvas();
  const Canvas &getCanvas() const;

  PreviewLayer &getPreview();
  const PreviewLayer &getPreview() const;

  void resize(int width, int height, const ResizePolicy &policy);

private:
  Canvas m_canvas;
  PreviewLayer m_preview;
};
