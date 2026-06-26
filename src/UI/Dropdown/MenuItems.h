#pragma once

#include "UI/Actions/MenuActions.h"
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  MenuItem
//
//  Pure data — no callbacks, no Editor references.
//  The menu system emits MenuAction IDs; MenuActionDispatcher
//  translates them into Editor calls.
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
  MenuAction action = MenuAction::None;
  bool enabled = true;
  bool checked = false;           // used when type == Checkbox
  std::vector<MenuItem> children; // used when type == Submenu

  // ── Factory helpers ───────────────────────────────────────────────────

  static MenuItem normal(std::string text, MenuAction action,
                         std::string shortcut = {}, bool enabled = true) {
    MenuItem m;
    m.type = MenuItemType::Normal;
    m.text = std::move(text);
    m.action = action;
    m.shortcut = std::move(shortcut);
    m.enabled = enabled;
    return m;
  }

  static MenuItem checkbox(std::string text, MenuAction action,
                           bool checked = false, bool enabled = true) {
    MenuItem m;
    m.type = MenuItemType::Checkbox;
    m.text = std::move(text);
    m.action = action;
    m.checked = checked;
    m.enabled = enabled;
    return m;
  }

  static MenuItem separator() {
    MenuItem m;
    m.type = MenuItemType::Separator;
    return m;
  }

  static MenuItem submenu(std::string text, std::vector<MenuItem> children) {
    MenuItem m;
    m.type = MenuItemType::Submenu;
    m.text = std::move(text);
    m.children = std::move(children);
    return m;
  }
};
