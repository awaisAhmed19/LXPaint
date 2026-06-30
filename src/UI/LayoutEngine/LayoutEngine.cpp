#include "LayoutEngine.h"
#include "UILayoutConstant.h"
#include <algorithm>
#include <cmath>

using namespace UI::Layout;

namespace {
constexpr int kToolbarButtonCount = 16;
constexpr float kToolbarOptionsBoxHeight = 84.f;
} // namespace

void LayoutEngine::update(int windowWidth, int windowHeight) {

  m_layout.ribbon = {0.f, 0.f, static_cast<float>(windowWidth), RibbonHeight};

  m_layout.toolbar = {0.f, RibbonHeight - 15.f, ToolbarWidth,
                      windowHeight - RibbonHeight - FooterHeight -
                          PaletteHeight};

  m_layout.palette = {ToolbarWidth, windowHeight - FooterHeight - PaletteHeight,
                      windowWidth - ToolbarWidth, PaletteHeight};

  m_layout.footer = {0.f, windowHeight - FooterHeight,
                     static_cast<float>(windowWidth), FooterHeight};

  m_layout.viewport = {ToolbarWidth, RibbonHeight, windowWidth - ToolbarWidth,
                       windowHeight - RibbonHeight - PaletteHeight -
                           FooterHeight};

  UI::ToolbarMetrics &tm = m_layout.toolbarMetrics;
  tm.columns = ToolbarColumns;
  tm.rows = (kToolbarButtonCount + tm.columns - 1) / tm.columns;
  tm.buttonGap = ItemSpacing;
  tm.leftInset = 4.f;

  const float gridAreaHeight =
      std::max(0.f, m_layout.toolbar.height - kToolbarOptionsBoxHeight);
  const float gridAreaWidth =
      std::max(0.f, m_layout.toolbar.width - 2.f * tm.leftInset);

  const float maxByWidth =
      (gridAreaWidth - tm.buttonGap * (tm.columns - 1)) / tm.columns;
  const float maxByHeight =
      (gridAreaHeight - tm.buttonGap * (tm.rows - 1)) / tm.rows;

  tm.buttonSize = std::clamp(std::min(maxByWidth, maxByHeight), 16.f, 32.f);

  tm.optionsBox = {
      m_layout.toolbar.x + 4.f,
      m_layout.toolbar.y + tm.buttonSize * tm.rows + tm.buttonGap * tm.rows +
          24.f,
      std::max(40.f, gridAreaWidth),
      kToolbarOptionsBoxHeight - 8.f,
  };

  tm.contentHeight =
      (tm.optionsBox.y - m_layout.toolbar.y) + tm.optionsBox.height + 4.f;
  tm.contentHeight = std::min(tm.contentHeight, m_layout.toolbar.height);
}
