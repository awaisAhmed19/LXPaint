#include "LayoutEngine.h"
void LayoutEngine::update(int windowWidth, int windowHeight, float ribbonHeight,
                          float toolbarWidth, float paletteHeight,
                          float footerHeight) {
  m_layout.ribbon = {0.f, 0.f, static_cast<float>(windowWidth), ribbonHeight};
  m_layout.footer = {0.f, static_cast<float>(windowHeight) - footerHeight,
                     static_cast<float>(windowWidth), footerHeight};
  m_layout.palette = {0.f, m_layout.footer.y - paletteHeight,
                      static_cast<float>(windowWidth), paletteHeight};
  m_layout.toolbar = {0.f, ribbonHeight, toolbarWidth,
                      m_layout.palette.y - ribbonHeight};
  m_layout.viewport = {toolbarWidth, ribbonHeight,
                       static_cast<float>(windowWidth) - toolbarWidth,
                       m_layout.palette.y - ribbonHeight};
}
