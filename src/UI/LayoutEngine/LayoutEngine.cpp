#include "LayoutEngine.h"
#include <iostream>
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
  std::cout << "\n========== LAYOUT ENGINE ==========\n";
  std::cout << "Window Size        : " << windowWidth << " x " << windowHeight
            << '\n';
  std::cout << "Ribbon Height      : " << ribbonHeight << '\n';
  std::cout << "Toolbar Width      : " << toolbarWidth << '\n';
  std::cout << "Palette Height     : " << paletteHeight << '\n';
  std::cout << "Footer Height      : " << footerHeight << '\n';

  std::cout << "Viewport Rect      : "
            << "X=" << m_layout.viewport.x << " Y=" << m_layout.viewport.y
            << " W=" << m_layout.viewport.width
            << " H=" << m_layout.viewport.height
            << "\n===================================\n";
}
