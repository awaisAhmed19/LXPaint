#include "TopRibbon.h"
#include "UI/Components/UIComponents.h"

TopRibbon::TopRibbon() { initializeMenus(); }

void TopRibbon::initializeMenus() {
  m_menus = {
      {"File", MenuType::File, [] {}},   {"Edit", MenuType::Edit, [] {}},
      {"View", MenuType::View, [] {}},   {"Image", MenuType::Image, [] {}},
      {"Color", MenuType::Color, [] {}}, {"Help", MenuType::Help, [] {}},
  };
}

void TopRibbon::bindMenu(MenuType type, std::function<void()> fn) {
  for (auto &entry : m_menus) {
    if (entry.type == type) {
      entry.onActivate = std::move(fn);
      return; // Found and updated, exit early
    }
  }
}

void TopRibbon::render() {
  ImGuiViewport *vp = ImGui::GetMainViewport();

  // Calculate ribbon dimensions
  const float fontHeight = ImGui::GetTextLineHeight();
  const float ribbonHeight = fontHeight + UIStyle::RibbonPaddingY * 2.5f;
  const float ribbonWidth = vp->Size.x;

  // Configure panel builder
  UI::PanelBuilder::config cfg;
  cfg.pos = vp->Pos;
  cfg.size = {ribbonWidth, ribbonHeight};
  cfg.pX = 0.f; // No horizontal padding (align to edges)
  cfg.pY = 0.f; // No vertical padding (align to edges)
  cfg.vertical = false;
  cfg.drawBorder = true;
  cfg.id = "##TopRibbonPanel";

  // Build and render ribbon
  UI::PanelBuilder builder(cfg);
  builder.drawBackground();

  // Add menu buttons
  for (auto &entry : m_menus) {
    UI::PanelBuilder::ItemConfig itemCfg;
    itemCfg.paddingX = UIStyle::PaddingX;
    itemCfg.paddingY = UIStyle::PaddingY;
    itemCfg.marginX = UIStyle::ItemSpacingX;
    itemCfg.marginY = 0.f;

    builder.Button(entry.label, itemCfg, [this, &entry]() {
      if (entry.onActivate) {
        entry.onActivate();
      }
    });
  }
}
