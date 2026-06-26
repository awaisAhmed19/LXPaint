#include "MenuActionDispatcher.h"

#include "Editor/Editor.h"
#include "Systems/Logger.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers — thin wrappers used below so the switch stays readable
// ─────────────────────────────────────────────────────────────────────────────

namespace {

void doUndo(Editor &editor) {
  // Editor::undo() now exists and forwards to CommandManager — this is
  // the same call path Ctrl+Z already uses via setupInputBindings().
  if (!editor.undo()) {
    Logger::log(LogLevel::DEBUG, "Edit Undo: nothing to undo");
  }
}

void doRedo(Editor &editor) {
  // Same as doUndo: now forwards to CommandManager via Editor::redo(),
  // matching the existing Ctrl+Y keyboard path.
  if (!editor.redo()) {
    Logger::log(LogLevel::DEBUG, "Edit Redo: nothing to redo");
  }
}

void doNew(Editor &editor) {
  // Resize to the default document size to simulate "New"
  // until a proper NewDocumentDialog is implemented.
  ResizePolicy policy;
  policy.anchor = ResizeAnchor::TOPLEFT;
  policy.fill = ResizeFill::BACKGROUNDCOLOR;
  editor.resizeCanvas(800, 500, policy);
}

void doInvertColors(Editor &editor) {
  Logger::warn("MenuAction::ImageInvertColors — not yet implemented");
  (void)editor;
}

void doSelectAll(Editor &editor) {
  Logger::warn("MenuAction::EditSelectAll — not yet implemented");
  (void)editor;
}

void doFlipRotate(Editor &editor) {
  Logger::warn("MenuAction::ImageFlipRotate — not yet implemented");
  (void)editor;
}

void doStretchSkew(Editor &editor) {
  Logger::warn("MenuAction::ImageStretchSkew — not yet implemented");
  (void)editor;
}

void doClear(Editor &editor) {
  Logger::warn("MenuAction::ImageClear — not yet implemented");
  (void)editor;
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
    doNew(editor);
    return true;

  case MenuAction::FileOpen:
    Logger::warn("MenuAction::FileOpen — file dialog not yet implemented");
    return true;

  case MenuAction::FileSave:
    Logger::warn("MenuAction::FileSave — not yet implemented");
    return true;

  case MenuAction::FileSaveAs:
    Logger::warn("MenuAction::FileSaveAs — not yet implemented");
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

    /*eventually expose something like:

  editor.requestQuit();

  or

  Application::requestQuit();

  Then the dispatcher doesn't know anything about SDL either.
    */
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
    Logger::warn("MenuAction::EditClearSelection — not yet implemented");
    return true;

  case MenuAction::EditSelectAll:
    doSelectAll(editor);
    return true;

  case MenuAction::EditCopyTo:
    Logger::warn("MenuAction::EditCopyTo — not yet implemented");
    return true;

  case MenuAction::EditPasteFrom:
    Logger::warn("MenuAction::EditPasteFrom — not yet implemented");
    return true;

  // ── View ─────────────────────────────────────────────────────────────
  case MenuAction::ViewToggleToolbox:
  case MenuAction::ViewToggleColorBox:
  case MenuAction::ViewToggleStatusBar:
  case MenuAction::ViewTextToolbar:
    Logger::warn("MenuAction::View toggle — not yet implemented");
    return true;

  case MenuAction::ViewZoom:
    Logger::warn("MenuAction::ViewZoom — not yet implemented");
    return true;

  case MenuAction::ViewBitmap:
    Logger::warn("MenuAction::ViewBitmap — not yet implemented");
    return true;

  case MenuAction::ViewFullscreen:
    Logger::warn("MenuAction::ViewFullscreen — not yet implemented");
    return true;

  // ── Image ─────────────────────────────────────────────────────────────
  case MenuAction::ImageFlipRotate:
    doFlipRotate(editor);
    return true;

  case MenuAction::ImageStretchSkew:
    doStretchSkew(editor);
    return true;

  case MenuAction::ImageInvertColors:
    doInvertColors(editor);
    return true;

  case MenuAction::ImageAttributes:
    Logger::warn("MenuAction::ImageAttributes — not yet implemented");
    return true;

  case MenuAction::ImageClear:
    doClear(editor);
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
