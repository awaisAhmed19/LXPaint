#include "LayoutEngine.h"
#include "UILayoutConstant.h"

using namespace UI::Layout;

void LayoutEngine::update(int windowWidth, int windowHeight) {

  m_layout.ribbon = {0.f, 0.f, static_cast<float>(windowWidth), RibbonHeight};

  m_layout.toolbar = {0.f, RibbonHeight, ToolbarWidth,
                      windowHeight - RibbonHeight - FooterHeight};

  m_layout.palette = {ToolbarWidth, windowHeight - FooterHeight - PaletteHeight,
                      windowWidth - ToolbarWidth, PaletteHeight};

  m_layout.footer = {0.f, windowHeight - FooterHeight,
                     static_cast<float>(windowWidth), FooterHeight};

  m_layout.viewport = {ToolbarWidth, RibbonHeight, windowWidth - ToolbarWidth,
                       windowHeight - RibbonHeight - PaletteHeight -
                           FooterHeight};
}
