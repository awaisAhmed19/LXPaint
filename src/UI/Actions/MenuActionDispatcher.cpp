#include "MenuActionDispatcher.h"

#include "Editor/Editor.h"
#include "Systems/Logger.h"
#include "UI/Dialogs/AppDialogs.h"
#include "UI/Dialogs/DialogManager.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Helper — open "Save Changes?" before a destructive document operation.
//
//  If the document is unmodified, `action` runs immediately.
//  If modified, the dialog runs, and `action` runs only if user chose Save
//  or Don't Save (not Cancel).
// ─────────────────────────────────────────────────────────────────────────────

namespace {

void guardedDocumentAction(Editor &editor,
                           std::function<void(bool saved)> action) {
  UI::DialogManager *mgr = editor.dialogManager();

  if (!editor.isModified() || !mgr) {
    // Nothing to save — run immediately (saved = false, just discard).
    action(false);
    return;
  }

  UI::AppDialogs::openSaveChanges(*mgr,
                                  [&editor, action](const UI::DialogResult &r) {
                                    if (r.buttonLabel == "Save") {
                                      editor.saveDocument();
                                      action(true);
                                    } else if (r.buttonLabel == "Don't Save") {
                                      action(false);
                                    }
                                    // "Cancel" or X: do nothing
                                  });
}

void doUndo(Editor &editor) {
  if (!editor.undo())
    Logger::log(LogLevel::DEBUG, "Edit Undo: nothing to undo");
}

void doRedo(Editor &editor) {
  if (!editor.redo())
    Logger::log(LogLevel::DEBUG, "Edit Redo: nothing to redo");
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
//  Dispatcher
// ─────────────────────────────────────────────────────────────────────────────

bool MenuActionDispatcher::execute(MenuAction action, Editor &editor) {
  UI::DialogManager *mgr = editor.dialogManager();

  switch (action) {

  // ── None ─────────────────────────────────────────────────────────────
  case MenuAction::None:
    return false;

    // ── File ─────────────────────────────────────────────────────────────

  case MenuAction::FileNew:
    guardedDocumentAction(editor,
                          [&editor](bool /*saved*/) { editor.newDocument(); });
    return true;

  case MenuAction::FileOpen:
    if (mgr) {
      guardedDocumentAction(editor, [&editor, mgr](bool /*saved*/) {
        UI::AppDialogs::openFileOpen(
            *mgr, [&editor](const std::string &path, bool accepted) {
              if (accepted && !path.empty()) {
                // TODO: use path once file-dialog is properly wired
                editor.openDocument();
              }
            });
      });
    } else {
      editor.openDocument();
    }
    return true;

  case MenuAction::FileSave:
    editor.saveDocument();
    return true;

  case MenuAction::FileSaveAs:
    if (mgr) {
      UI::AppDialogs::openFileSaveAs(
          *mgr, "", [&editor](const std::string &path, bool accepted) {
            if (accepted) {
              // TODO: pass path through to ImageIO once Save As is
              // fully wired; for now fall back to existing behaviour.
              editor.saveDocumentAs();
            }
          });
    } else {
      editor.saveDocumentAs();
    }
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
    if (mgr) {
      // TODO: read current zoom from editor viewport and pass it
      UI::AppDialogs::openCustomZoom(
          *mgr, 1.0f, [](const UI::AppDialogs::ZoomResult &r) {
            if (r.accepted) {
              Logger::debug(std::format("Zoom set to {}%", r.zoomPercent));
              // TODO: forward zoom to editor viewport
            }
          });
    }
    return true;

  case MenuAction::ViewBitmap:
    Logger::warn("MenuAction::ViewBitmap — not yet implemented");
    return true;

  case MenuAction::ViewFullscreen:
    editor.setFullscreen(!editor.isFullscreen());
    return true;

  // ── Image ─────────────────────────────────────────────────────────────
  case MenuAction::ImageFlipRotate:
    if (mgr) {
      UI::AppDialogs::openFlipRotate(
          *mgr, [&editor](const UI::AppDialogs::FlipRotateResult &r) {
            if (!r.accepted)
              return;
            switch (r.choice) {
            case UI::AppDialogs::FlipRotateChoice::FlipHorizontal:
              editor.flipHorizontal();
              break;
            case UI::AppDialogs::FlipRotateChoice::FlipVertical:
              editor.flipVertical();
              break;
            case UI::AppDialogs::FlipRotateChoice::RotateByAngle:
              // Snap to nearest 90° increment
              if (r.angleDeg <= 135.f)
                editor.rotate90CW();
              else if (r.angleDeg <= 225.f) {
                editor.rotate90CW();
                editor.rotate90CW();
              } else
                editor.rotate90CCW();
              break;
            }
          });
    } else {
      editor.flipHorizontal();
    }
    return true;

  case MenuAction::ImageStretchSkew:
    if (mgr) {
      UI::AppDialogs::openStretchSkew(
          *mgr, [](const UI::AppDialogs::StretchSkewResult &r) {
            if (!r.accepted)
              return;
            Logger::warn(
                "Stretch/Skew accepted — operation not yet implemented");
          });
    }
    return true;

  case MenuAction::ImageInvertColors:
    editor.invertColors();
    return true;

  case MenuAction::ImageAttributes:
    if (mgr) {
      int cw = 800, ch = 500; // TODO: read from editor document
      UI::AppDialogs::openAttributes(
          *mgr, cw, ch, [&editor](const UI::AppDialogs::AttributesResult &r) {
            if (!r.accepted)
              return;
            ResizePolicy policy;
            policy.anchor = ResizeAnchor::TOPLEFT;
            policy.fill = ResizeFill::BACKGROUNDCOLOR;
            editor.resizeCanvas(r.width, r.height, policy);
          });
    }
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
