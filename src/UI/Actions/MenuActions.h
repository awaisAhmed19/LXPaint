#pragma once

enum class MenuAction {
    None,

    // ── File ─────────────────────────────────────────────
    FileNew,
    FileOpen,
    FileSave,
    FileSaveAs,

    FileLoadURL,
    FileUploadImgur,
    FileManageStorage,

    FilePrintPreview,
    FilePageSetup,
    FilePrint,

    FileWallpaperTiled,
    FileWallpaperCentered,

    FileExit,

    // ── Edit ─────────────────────────────────────────────
    EditUndo,
    EditRedo,
    EditHistory,

    EditCut,
    EditCopy,
    EditPaste,
    EditClearSelection,
    EditSelectAll,

    EditCopyTo,
    EditPasteFrom,

    // ── View ─────────────────────────────────────────────
    ViewToggleToolbox,
    ViewToggleColorBox,
    ViewToggleStatusBar,
    ViewTextToolbar,

    ViewZoom,
    ViewBitmap,
    ViewFullscreen,

    // ── Image ────────────────────────────────────────────
    ImageFlipRotate,
    ImageStretchSkew,

    ImageInvertColors,
    ImageAttributes,
    ImageClear,
    ImageDrawOpaque,

    // ── Colors ───────────────────────────────────────────
    ColorsEdit,
    ColorsGet,
    ColorsSave,
};
