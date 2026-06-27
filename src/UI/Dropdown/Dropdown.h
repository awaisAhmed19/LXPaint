#pragma once

#include "MenuItems.h"
#include "UI/Actions/MenuActions.h"
#include "UI/LayoutEngine/DropdownMetrics.h"
#include "imgui.h"

#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  Dropdown
//
//  A single Windows-95-style popup menu panel.
//
//  hoverKey (constructor param) is the FooterMessages key shown when the
//  user hovers the ribbon button for this menu — e.g. nothing classical
//  exists for "File" as a button, so it defaults to empty (no message).
//  Individual menu items carry their own hoverKey in MenuItem::hoverKey.
// ─────────────────────────────────────────────────────────────────────────────

class Dropdown {
public:
  explicit Dropdown(std::string title, std::vector<MenuItem> items,
                    std::string hoverKey = {});

  // ── State ─────────────────────────────────────────────────────────────

  bool isOpen() const { return m_open; }
  void open() {
    m_open = true;
    m_hoveredIndex = -1;
  }
  void close() {
    m_open = false;
    m_hoveredIndex = -1;
  }
  void toggle() { m_open ? close() : open(); }

  const std::string &title() const { return m_title; }

  void setChecked(size_t index, bool checked);

  // ── Rendering ─────────────────────────────────────────────────────────

  bool renderRibbonButton(ImDrawList *dl, ImVec2 btnMin, ImVec2 btnMax,
                          bool isActive);
  MenuAction renderPanel(ImVec2 originPos);
  bool contains(ImVec2 origin, ImVec2 mouse) const;

private:
  float computePanelWidth() const;
  float computePanelHeight() const;

  MenuAction renderItem(ImDrawList *dl, const MenuItem &item, ImVec2 rowMin,
                        float panelWidth, int index);
  void renderSeparator(ImDrawList *dl, ImVec2 rowMin, float panelWidth);
  void renderCheckmark(ImDrawList *dl, ImVec2 rowMin) const;
  void renderSubmenuArrow(ImDrawList *dl, ImVec2 rowMin,
                          float panelWidth) const;

  void drawRaisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const;
  void drawSunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const;

  // ── Data ──────────────────────────────────────────────────────────────

  std::string m_title;
  std::string m_hoverKey; // shown when the ribbon button is hovered
  std::vector<MenuItem> m_items;
  bool m_open = false;
  int m_hoveredIndex = -1;
  int m_openSubmenu = -1;
};
