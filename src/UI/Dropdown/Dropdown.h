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
//  Responsibilities:
//    • Store and render one flat or nested list of MenuItems.
//    • Track open/closed state and hovered row.
//    • Hit-test clicks and return the chosen MenuAction.
//    • Never call into the Editor.
//
//  Usage (Ribbon manages a list of these):
//
//    Dropdown d("File", buildFileMenu());
//
//    // Inside render loop, after drawing the ribbon button:
//    if (d.isOpen())
//        MenuAction act = d.renderPanel(originPos);
//        // act != None means user clicked something
//
// ─────────────────────────────────────────────────────────────────────────────

class Dropdown {
public:
  explicit Dropdown(std::string title, std::vector<MenuItem> items);

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

  // Allow external code (e.g. ImageAttributes checkbox) to sync checked state
  void setChecked(size_t index, bool checked);

  // ── Rendering ─────────────────────────────────────────────────────────

  // Draw the ribbon label button.  Returns true if clicked.
  // anchorMin/anchorMax are the screen-space rect of the button (used to
  // position the popup directly below it).
  bool renderRibbonButton(ImDrawList *dl, ImVec2 btnMin, ImVec2 btnMax,
                          bool isActive);

  // Draw the open popup panel, starting at originPos (typically the
  // bottom-left of the ribbon button).  Returns the action the user
  // clicked, or MenuAction::None.
  MenuAction renderPanel(ImVec2 originPos);
  bool contains(ImVec2 origin, ImVec2 mouse) const;

private:
  // ── Internal rendering helpers ────────────────────────────────────────

  float computePanelWidth() const;
  float computePanelHeight() const;

  // Render one row.  Returns the action if clicked, None otherwise.
  MenuAction renderItem(ImDrawList *dl, const MenuItem &item, ImVec2 rowMin,
                        float panelWidth, int index);

  void renderSeparator(ImDrawList *dl, ImVec2 rowMin, float panelWidth);
  void renderCheckmark(ImDrawList *dl, ImVec2 rowMin) const;
  void renderSubmenuArrow(ImDrawList *dl, ImVec2 rowMin,
                          float panelWidth) const;
  // Classic Win95 raised / sunken panel border
  void drawRaisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const;
  void drawSunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const;

  // ── Data ──────────────────────────────────────────────────────────────

  std::string m_title;
  std::vector<MenuItem> m_items;
  bool m_open = false;
  int m_hoveredIndex = -1;

  // Submenu open state: index into m_items of the currently open submenu,
  // -1 if none.
  int m_openSubmenu = -1;
};
