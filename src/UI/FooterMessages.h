#pragma once

#include <string>
#include <unordered_map>

// ─────────────────────────────────────────────────────────────────────────────
//  FooterMessages
//
//  Single source of truth for every status-bar help string.
//  Keys mirror the classic MS Paint JS lookup table.
//  No other file should contain these descriptive strings.
//
//  Usage:
//    const std::string* msg = FooterMessages::get("pencil");
//    if (msg) footer.setText(*msg);
// ─────────────────────────────────────────────────────────────────────────────

namespace FooterMessages {

// Message key constants — use these so callers never type raw strings.
namespace Key {

// ── Tools ────────────────────────────────────────────────────────────────────
inline constexpr const char *FreeSelect = "polygonlasso";
inline constexpr const char *RectSelect = "rectlasso";
inline constexpr const char *Eraser = "eraser";
inline constexpr const char *FloodFill = "fill";
inline constexpr const char *Eyedropper = "eyedrop";
inline constexpr const char *Magnifier = "zoom";
inline constexpr const char *Pencil = "pencil";
inline constexpr const char *Brush = "brush";
inline constexpr const char *Airbrush = "airbrush";
inline constexpr const char *Text = "text";
inline constexpr const char *Line = "line";
inline constexpr const char *Curve = "curveline";
inline constexpr const char *Rectangle = "rectshape";
inline constexpr const char *Polygon = "polygonshape";
inline constexpr const char *Ellipse = "elipse";
inline constexpr const char *RoundedRect = "rectelipse";

// ── File menu ─────────────────────────────────────────────────────────────
inline constexpr const char *FileNew = "New_file";
inline constexpr const char *FileOpen = "Open_file";
inline constexpr const char *FileSave = "Save_file";
inline constexpr const char *FileSaveAs = "Saveas_file";
inline constexpr const char *FileLoadURL = "Load_from_URL";
inline constexpr const char *FileUploadImgur = "Upload_to_Imgur";
inline constexpr const char *FileManageStorage = "Manage_storage";
inline constexpr const char *FileWallpaperTiled = "Set_as_WallpaperT";
inline constexpr const char *FileWallpaperCentr = "Set_as_WallpaperC";
inline constexpr const char *FileRecentFiles = "Recent_files";
inline constexpr const char *FileExit = "Exit";

// ── Edit menu ─────────────────────────────────────────────────────────────
inline constexpr const char *EditUndo = "Undo";
inline constexpr const char *EditRedo = "Repeat";
inline constexpr const char *EditHistory = "History";
inline constexpr const char *EditCut = "Cut";
inline constexpr const char *EditCopy = "Copy";
inline constexpr const char *EditPaste = "Paste";
inline constexpr const char *EditClearSelection = "Clear_Selection";
inline constexpr const char *EditSelectAll = "Select_All";
inline constexpr const char *EditCopyTo = "Copy_To";
inline constexpr const char *EditPasteFrom = "Paste_from";

// ── View menu ─────────────────────────────────────────────────────────────
inline constexpr const char *ViewToolbox = "Tool_Box";
inline constexpr const char *ViewColorBox = "Color_Box";
inline constexpr const char *ViewStatusBar = "Status_Bar";
inline constexpr const char *ViewTextBar = "Text_Toolbar";
inline constexpr const char *ViewZoom = "Zoom";
inline constexpr const char *ViewBitmap = "View_Bitmap";
inline constexpr const char *ViewFullscreen = "FullScreen";

// ── Image menu ────────────────────────────────────────────────────────────
inline constexpr const char *ImageFlipRotate = "Flip_Rotate";
inline constexpr const char *ImageStretchSkew = "Stretch_Skew";
inline constexpr const char *ImageInvert = "Invert_Color";
inline constexpr const char *ImageAttributes = "Attributes";
inline constexpr const char *ImageClear = "Clear_Image";
inline constexpr const char *ImageDrawOpaque = "Draw_Opaque";

// ── Colors menu ───────────────────────────────────────────────────────────
inline constexpr const char *ColorsEdit = "Edit_Colors";
inline constexpr const char *ColorsGet = "Get_Colors";
inline constexpr const char *ColorsSave = "Save_Colors";

} // namespace Key

// ─────────────────────────────────────────────────────────────────────────────
//  Lookup
// ─────────────────────────────────────────────────────────────────────────────

inline const std::unordered_map<std::string, std::string> &table() {
  static const std::unordered_map<std::string, std::string> kTable = {
      // Tools
      {"polygonlasso",
       "Selects a free-form of the picture to move, copy, or edit."},
      {"rectlasso",
       "Selects a rectangular part of the picture to move, copy or edit."},
      {"eraser",
       "Erases a portion of the picture, using the selected eraser shape."},
      {"fill", "Fills an area with the selected drawing color."},
      {"eyedrop", "Picks up a color from the picture for drawing."},
      {"zoom", "Changes the magnification."},
      {"pencil", "Draws a free-form line one pixel wide."},
      {"brush", "Draws using a brush with the selected shape and size."},
      {"airbrush", "Draws using an airbrush of the selected size."},
      {"text", "Inserts text into the picture."},
      {"line", "Draws a straight line with the selected line width."},
      {"curveline", "Draws a curved line with the selected line width."},
      {"rectshape", "Draws a rectangle with the selected fill style."},
      {"polygonshape", "Draws a polygon with the selected fill style."},
      {"elipse", "Draws an ellipse with the selected fill style."},
      {"rectelipse", "Draws a rounded rectangle with the selected fill style."},
      // File
      {"New_file", "Creates a new document."},
      {"Open_file", "Opens an existing document."},
      {"Save_file", "Saves the active document."},
      {"Saveas_file", "Saves the active document with a new name."},
      {"Load_from_URL", "Opens an image from the Web."},
      {"Upload_to_Imgur", "Uploads the active document on Imgur."},
      {"Manage_storage",
       "Manage storage of previously created or open documents."},
      {"Set_as_WallpaperT", "Tiles this bitmap on the desktop background."},
      {"Set_as_WallpaperC", "Centers this bitmap on the desktop background."},
      {"Recent_files", "List recent files."},
      {"Exit", "Quits Paint."},
      // Edit
      {"Undo", "Undoes the last action."},
      {"Repeat", "Repeats the last undone action."},
      {"History", "Shows the action history."},
      {"Cut", "Cuts the selection and puts it on the clipboard."},
      {"Copy", "Copies the selection and puts it on the clipboard."},
      {"Paste", "Inserts the contents of the clipboard."},
      {"Clear_Selection", "Deletes the selection."},
      {"Select_All", "Selects everything."},
      {"Copy_To", "Copies the selection to a file."},
      {"Paste_from", "Pastes a file into the selection."},
      // View
      {"Tool_Box", "Shows or hides the toolbar."},
      {"Color_Box", "Shows or hides the color bar."},
      {"Status_Bar", "Shows or hides the status bar."},
      {"Text_Toolbar", "Shows or hides the text toolbar."},
      {"Zoom", "Zooming with options."},
      {"View_Bitmap", "Displays the entire picture."},
      {"FullScreen", "Makes the application full screen."},
      // Image
      {"Flip_Rotate", "Flips or rotates a picture or a selection."},
      {"Stretch_Skew", "Stretches or skews a picture or a selection."},
      {"Invert_Color", "Inverts the colors of a picture or a selection."},
      {"Attributes", "Changes the attributes of a picture."},
      {"Clear_Image", "Clears the canvas."},
      {"Draw_Opaque",
       "Makes the current selection either opaque or transparent."},
      // Colors
      {"Edit_Colors", "Creates a new color."},
      {"Get_Colors", "Uses a previously saved palette of colors."},
      {"Save_Colors", "Saves the current palette of colors to a file."},
  };
  return kTable;
}

// Returns nullptr if the key is unknown — callers should handle gracefully.
inline const std::string *get(const std::string &key) {
  auto it = table().find(key);
  if (it == table().end())
    return nullptr;
  return &it->second;
}

} // namespace FooterMessages
