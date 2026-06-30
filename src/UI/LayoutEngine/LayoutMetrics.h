#pragma once
namespace UI {

inline constexpr float ItemSpacing = 1.f;

struct PanelRect {
  float x;
  float y;
  float width;
  float height;
};

struct ToolbarMetrics {
  int columns = 2;
  int rows = 8;
  float buttonSize = 24.f;
  float buttonGap = 1.f;
  float leftInset = 4.f;
  float contentHeight = 0.f;
  PanelRect optionsBox{};
};

struct LayoutMetrics {
  PanelRect ribbon;
  PanelRect toolbar;
  PanelRect palette;
  PanelRect footer;
  PanelRect viewport;
  ToolbarMetrics toolbarMetrics;
};
}; // namespace UI
