#pragma once

#include "LayoutMetrics.h"

class LayoutEngine {
public:
  void update(int windowWidth, int windowHeight, float ribbonHeight,
              float toolbarWidth, float paletteHeight, float footerHeight);

  const UI::LayoutMetrics &layout() const { return m_layout; }

private:
  UI::LayoutMetrics m_layout;
};
