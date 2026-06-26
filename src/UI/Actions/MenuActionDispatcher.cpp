#include "MenuActionDispatcher.h"

#include "Editor/Editor.h"
#include "Systems/Logger.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers — thin wrappers used below so the switch stays readable
// ─────────────────────────────────────────────────────────────────────────────

namespace {

void doUndo(Editor &editor) {
  if (!editor.undo()) {
    Logger::log(LogLevel::DEBUG, "Edit Undo: nothing to undo");
  }
}

void doRedo(Editor &editor) {
  if (!editor.redo()) {
    Logger::log(LogLevel::DEBUG, "Edit Redo: nothing to redo");
  }
}

void doNew(Editor &editor) {
  ResizePolicy policy;
  policy.anchor = ResizeAnchor::TOPLEFT;
  policy.fill = ResizeFill::BACKGROUNDCOLOR;
  editor.resizeCanvas(800, 500, policy);
  // first check if the undo is empty
  // if it is then there was no interaction that happened in the canvas
  // if there was interation then we have to first save/ saveas the canvas
  // clear the canvas
  // resize to default
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
//  Dispatcher
// ─────────────────────────────────────────────────────────────────────────────

bool MenuActionDispatcher::execute(MenuAction action, Editor &editor) {
  switch (action) {

  // ── None ─────────────────────────────────────────────────────────────
  case MenuAction::None:
    return false;

    // ── File ─────────────────────────────────────────────────────────────
  case MenuAction::FileNew:
    editor.newDocument();
    return true;

  case MenuAction::FileOpen:
    editor.openDocument();
    return true;

  case MenuAction::FileSave:
    editor.saveDocument();
    return true;

  case MenuAction::FileSaveAs:
    editor.saveDocumentAs();
    return true;

  case MenuAction::FileLoadURL:
    Logger::warn("MenuAction::FileLoadURL — not yet implemented");
    return true;

  case MenuAction::FileUploadImgur:
    Logger::warn("MenuAction::FileUploadImgur — not yet implemented");
    return true;

  case MenuAction::FileManageStorage:
    Logger::warn("MenuAction::FileManageStorage — not yet implemented");
    return true;

  case MenuAction::FilePrintPreview:
    Logger::warn("MenuAction::FilePrintPreview — not yet implemented");
    return true;

  case MenuAction::FilePageSetup:
    Logger::warn("MenuAction::FilePageSetup — not yet implemented");
    return true;

  case MenuAction::FilePrint:
    Logger::warn("MenuAction::FilePrint — not yet implemented");
    return true;

  case MenuAction::FileWallpaperTiled:
    Logger::warn("MenuAction::FileWallpaperTiled — not yet implemented");
    return true;

  case MenuAction::FileWallpaperCentered:
    Logger::warn("MenuAction::FileWallpaperCentered — not yet implemented");
    return true;

  case MenuAction::FileExit: {
    SDL_Event quit{};
    quit.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&quit);
    return true;
  }

  // ── Edit ─────────────────────────────────────────────────────────────
  case MenuAction::EditUndo:
    doUndo(editor);
    return true;

  case MenuAction::EditRedo:
    doRedo(editor);
    return true;

  case MenuAction::EditHistory:
    Logger::warn("MenuAction::EditHistory — not yet implemented");
    return true;

  case MenuAction::EditCut:
    Logger::warn("MenuAction::EditCut — not yet implemented");
    return true;

  case MenuAction::EditCopy:
    Logger::warn("MenuAction::EditCopy — not yet implemented");
    return true;

  case MenuAction::EditPaste:
    Logger::warn("MenuAction::EditPaste — not yet implemented");
    return true;

  case MenuAction::EditClearSelection:
    editor.clearSelection();
    return true;

  case MenuAction::EditSelectAll:
    editor.selectAll();
    return true;

  case MenuAction::EditCopyTo:
    Logger::warn("MenuAction::EditCopyTo — not yet implemented");
    return true;

  case MenuAction::EditPasteFrom:
    Logger::warn("MenuAction::EditPasteFrom — not yet implemented");
    return true;

  // ── View ─────────────────────────────────────────────────────────────
  case MenuAction::ViewToggleToolbox:
    editor.setToolboxVisible(!editor.isToolboxVisible());
    return true;

  case MenuAction::ViewToggleColorBox:
    editor.setPaletteVisible(!editor.isPaletteVisible());
    return true;

  case MenuAction::ViewToggleStatusBar:
    editor.setStatusBarVisible(!editor.isStatusBarVisible());
    return true;

  case MenuAction::ViewTextToolbar:
    Logger::warn("MenuAction::ViewTextToolbar — not yet implemented");
    return true;

  case MenuAction::ViewZoom:
    Logger::warn("MenuAction::ViewZoom — not yet implemented");
    return true;

  case MenuAction::ViewBitmap:
    Logger::warn("MenuAction::ViewBitmap — not yet implemented");
    return true;

  case MenuAction::ViewFullscreen:
    editor.setFullscreen(!editor.isFullscreen());
    return true;

  // ── Image ─────────────────────────────────────────────────────────────
  case MenuAction::ImageFlipRotate:
    // The Ribbon's "Flip/Rotate" entry is a single menu item standing in
    // for what classic MS Paint shows as a sub-dialog (flip horizontal /
    // flip vertical / rotate by angle). MenuItems.h has no submenu wired
    // here yet (see Ribbon::buildImageMenu) — until that dialog/submenu
    // exists, route the single entry to the most common case (flip
    // horizontal) rather than leaving it a no-op. Rotate90CW/CCW are
    // already separately callable via Editor for when the UI grows
    // dedicated entries.
    editor.flipHorizontal();
    return true;

  case MenuAction::ImageStretchSkew:
    Logger::warn("MenuAction::ImageStretchSkew — not yet implemented");
    return true;

  case MenuAction::ImageInvertColors:
    editor.invertColors();
    return true;

  case MenuAction::ImageAttributes:
    Logger::warn("MenuAction::ImageAttributes — not yet implemented");
    return true;

  case MenuAction::ImageClear:
    editor.clearImage();
    return true;

  case MenuAction::ImageDrawOpaque:
    Logger::warn("MenuAction::ImageDrawOpaque — not yet implemented");
    return true;

  // ── Colors ────────────────────────────────────────────────────────────
  case MenuAction::ColorsEdit:
    Logger::warn("MenuAction::ColorsEdit — not yet implemented");
    return true;

  case MenuAction::ColorsGet:
    Logger::warn("MenuAction::ColorsGet — not yet implemented");
    return true;

  case MenuAction::ColorsSave:
    Logger::warn("MenuAction::ColorsSave — not yet implemented");
    return true;
  }

  return false;
}
