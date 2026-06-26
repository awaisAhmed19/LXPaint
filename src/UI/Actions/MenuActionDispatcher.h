#pragma once

#include "MenuActions.h"

class Editor;

// ─────────────────────────────────────────────────────────────────────────────
//  MenuActionDispatcher
//
//  The single translation layer between the UI and the Editor.
//  UI emits a MenuAction; this class maps it to an Editor call.
//
//  Nothing in Ribbon, Dropdown, or MenuItem ever includes Editor.h.
//  All Editor knowledge lives here.
//
//  Future: replace the switch with a command factory that pushes onto
//  CommandManager, giving every menu action free undo/redo.
// ─────────────────────────────────────────────────────────────────────────────

class MenuActionDispatcher {
public:
  // Execute the action against the editor.
  // Returns true if the action was handled (even if as a no-op for
  // currently-unimplemented items), false only for MenuAction::None.
  static bool execute(MenuAction action, Editor &editor);
};
