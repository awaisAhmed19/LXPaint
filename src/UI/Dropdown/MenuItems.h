#pragma once

#include "UI/Actions/MenuActions.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  MenuItem
//
//  Pure data — no callbacks, no Editor references.
//  hoverKey is a FooterMessages::Key constant (or raw key string).
//  Empty string means "no footer message for this item".
// ─────────────────────────────────────────────────────────────────────────────

enum class MenuItemType {
  Normal,
  Separator,
  Checkbox,
  Submenu,
};

struct MenuItem {
  MenuItemType type = MenuItemType::Normal;
  std::string text;
  std::string shortcut;
  std::string hoverKey; // FooterMessages lookup key; empty = no message
  MenuAction action = MenuAction::None;
  bool enabled = true;
  bool checked = false;
  std::vector<MenuItem> children; // used when type == Submenu

  // ── Factory helpers ───────────────────────────────────────────────────

  static MenuItem normal(std::string text, MenuAction action,
                         std::string shortcut = {}, bool enabled = true,
                         std::string hoverKey = {}) {
    MenuItem m;
    m.type = MenuItemType::Normal;
    m.text = std::move(text);
    m.action = action;
    m.shortcut = std::move(shortcut);
    m.enabled = enabled;
    m.hoverKey = std::move(hoverKey);
    return m;
  }

  static MenuItem checkbox(std::string text, MenuAction action,
                           bool checked = false, bool enabled = true,
                           std::string hoverKey = {}) {
    MenuItem m;
    m.type = MenuItemType::Checkbox;
    m.text = std::move(text);
    m.action = action;
    m.checked = checked;
    m.enabled = enabled;
    m.hoverKey = std::move(hoverKey);
    return m;
  }

  static MenuItem separator() {
    MenuItem m;
    m.type = MenuItemType::Separator;
    return m;
  }

  static MenuItem submenu(std::string text, std::vector<MenuItem> children,
                          std::string hoverKey = {}) {
    MenuItem m;
    m.type = MenuItemType::Submenu;
    m.text = std::move(text);
    m.children = std::move(children);
    m.hoverKey = std::move(hoverKey);
    return m;
  }
};
