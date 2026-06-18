#pragma once
namespace UI {

inline constexpr float ItemSpacing = 1.f;
struct PanelRect {
  float x;
  float y;
  float width;
  float height;
};

struct LayoutMetrics {
  PanelRect ribbon;
  PanelRect toolbar;
  PanelRect palette;
  PanelRect footer;
  PanelRect viewport;
};
}; // namespace UI
