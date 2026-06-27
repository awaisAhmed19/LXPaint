#pragma once
#include "Dialog.h"
#include "DialogControl.h"
#include "DialogManager.h"

#include "App/Globals.h"

// ─────────────────────────────────────────────────────────────────────────────
//  AppDialogs
//
//  Factory functions for every dialog in the application.
//  Each function configures a Dialog via the generic API and opens it.
//  No dialog rendering code lives here — only configuration.
//
//  All dialog results are delivered through a callback so callers remain
//  decoupled from the dialog lifetime.
// ─────────────────────────────────────────────────────────────────────────────

class Editor;

namespace UI::AppDialogs {

// ─────────────────────────────────────────────────────────────────────────────
//  File → New  ("Save Changes?")
//
//  Shown when the user triggers New but the document has unsaved changes.
//  Callback receives:
//    buttonLabel == "Save"        → caller should save then create new doc
//    buttonLabel == "Don't Save"  → caller should create new doc immediately
//    buttonLabel == "Cancel"      → abort; do nothing
// ─────────────────────────────────────────────────────────────────────────────

inline void
openSaveChanges(DialogManager &mgr,
                std::function<void(const DialogResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("LXPaint");
  d.setWidth(300.f);

  d.addLabel("The image has been modified.\nDo you want to save changes?");

  d.addButton("Save", true, false);
  d.addButton("Don't Save", false, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, std::move(callback));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Image → Attributes
//
//  Width / Height inputs + Units radio + Colors radio + Transparency radio.
//
//  Returns via callback. Caller inspects:
//    result.buttonLabel == "OK" → read widthCtrl / heightCtrl values via
//    stored raw pointers (captured in the lambda before opening).
// ─────────────────────────────────────────────────────────────────────────────

struct AttributesResult {
  int width = 800;
  int height = 500;
  int units = 2;        // 0=Inches 1=Cm 2=Pixels
  int colors = 1;       // 0=B&W    1=Colors
  int transparency = 1; // 0=Transparent 1=Opaque
  bool accepted = false;
};

inline void
openAttributes(DialogManager &mgr, int currentW, int currentH,
               std::function<void(const AttributesResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Attributes");
  d.setWidth(380.f);

  // Info labels
  d.addLabel("File last saved:  Not Available");
  d.addLabel("Size on disk:     Not Available");
  d.addLabel("Resolution:       72 x 72 dots per inch");

  d.addSeparator();

  // Width / Height on same row — two IntFields side by side
  // We achieve this with two IntFieldControls using short label widths
  auto *wCtrl = d.addIntField("Width:", currentW, "", 45.f, 50.f);
  auto *hCtrl = d.addIntField("Height:", currentH, "", 45.f, 50.f);

  // Units group box
  auto *unitsBox = d.addGroupBox("Units");
  auto *unitsRG = unitsBox->add<RadioGroupControl>(
      std::vector<RadioGroupControl::Option>{{"Inches"}, {"Cm"}, {"Pixels"}},
      2 /* Pixels default */);

  // Colors group box
  auto *colorsBox = d.addGroupBox("Colors");
  auto *colorsRG = colorsBox->add<RadioGroupControl>(
      std::vector<RadioGroupControl::Option>{{"Black and white"}, {"Colors"}},
      1 /* Colors default */);

  // Transparency group box
  auto *transBox = d.addGroupBox("Transparency");
  auto *transRG = transBox->add<RadioGroupControl>(
      std::vector<RadioGroupControl::Option>{{"Transparent"}, {"Opaque"}},
      1 /* Opaque default */);

  d.addButton("OK", true, false);
  d.addButton("Cancel", false, true);
  d.addButton("Default", false, false);

  mgr.open(d, [=](const DialogResult &r) {
    AttributesResult ar;
    ar.width = wCtrl->getValue();
    ar.height = hCtrl->getValue();
    ar.units = unitsRG->getSelected();
    ar.colors = colorsRG->getSelected();
    ar.transparency = transRG->getSelected();
    ar.accepted = (r.buttonLabel == "OK");
    callback(ar);
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Image → Flip and Rotate
// ─────────────────────────────────────────────────────────────────────────────

enum class FlipRotateChoice {
  FlipHorizontal = 0,
  FlipVertical = 1,
  RotateByAngle = 2
};

struct FlipRotateResult {
  FlipRotateChoice choice = FlipRotateChoice::FlipHorizontal;
  float angleDeg = 90.f;
  bool accepted = false;
};

inline void
openFlipRotate(DialogManager &mgr,
               std::function<void(const FlipRotateResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Flip and Rotate");
  d.setWidth(280.f);

  auto *box = d.addGroupBox("Flip or rotate");
  auto *rg = box->add<RadioGroupControl>(
      std::vector<RadioGroupControl::Option>{
          {"Flip horizontal"}, {"Flip vertical"}, {"Rotate by angle"}},
      0);

  // Sub-radio for angle
  box->add<RadioGroupControl>(
      std::vector<RadioGroupControl::Option>{{"90°"}, {"180°"}, {"270°"}}, 0,
      16.f /* indent */);

  // Degrees field
  auto *angleField =
      box->add<FloatFieldControl>("", 90.f, "Degrees", 4.f, 40.f);

  d.addButton("OK", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    FlipRotateResult fr;
    fr.choice = static_cast<FlipRotateChoice>(rg->getSelected());
    fr.angleDeg = (fr.choice == FlipRotateChoice::RotateByAngle)
                      ? angleField->getValue()
                      : 0.f;
    fr.accepted = (r.buttonLabel == "OK");
    callback(fr);
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Image → Stretch and Skew
// ─────────────────────────────────────────────────────────────────────────────

struct StretchSkewResult {
  float stretchH = 100.f; // percent
  float stretchV = 100.f; // percent
  float skewH = 0.f;      // degrees
  float skewV = 0.f;      // degrees
  bool accepted = false;
};

inline void
openStretchSkew(DialogManager &mgr,
                std::function<void(const StretchSkewResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Stretch and Skew");
  d.setWidth(300.f);

  auto *stretchBox = d.addGroupBox("Stretch");
  auto *shCtrl =
      stretchBox->add<FloatFieldControl>("Horizontal:", 100.f, "%", 80.f, 50.f);
  auto *svCtrl =
      stretchBox->add<FloatFieldControl>("Vertical:", 100.f, "%", 80.f, 50.f);

  auto *skewBox = d.addGroupBox("Skew");
  auto *khCtrl = skewBox->add<FloatFieldControl>("Horizontal:", 0.f, "Degrees",
                                                 80.f, 40.f);
  auto *kvCtrl =
      skewBox->add<FloatFieldControl>("Vertical:", 0.f, "Degrees", 80.f, 40.f);

  d.addButton("OK", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    StretchSkewResult sr;
    sr.stretchH = shCtrl->getValue();
    sr.stretchV = svCtrl->getValue();
    sr.skewH = khCtrl->getValue();
    sr.skewV = kvCtrl->getValue();
    sr.accepted = (r.buttonLabel == "OK");
    callback(sr);
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  View → Zoom (custom zoom level)
// ─────────────────────────────────────────────────────────────────────────────

struct ZoomResult {
  float zoomPercent = 100.f;
  bool accepted = false;
};

inline void openCustomZoom(DialogManager &mgr, float currentZoom,
                           std::function<void(const ZoomResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Custom Zoom");
  d.setWidth(240.f);

  d.addLabel("Zoom (percentage):");
  auto *zCtrl = d.addFloatField("", currentZoom * 100.f, "%", 4.f, 60.f);

  d.addButton("OK", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    ZoomResult zr;
    zr.zoomPercent = zCtrl->getValue();
    zr.accepted = (r.buttonLabel == "OK");
    callback(zr);
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Help → About
// ─────────────────────────────────────────────────────────────────────────────

inline void openAbout(DialogManager &mgr) {
  Dialog &d = mgr.create();
  d.setTitle("About LXPaint");
  d.setWidth(340.f);

  d.addLabel("LXPaint");
  d.addLabel("Version 1.0.0");
  d.addSeparator();
  d.addLabel("A classic raster graphics editor\nbuilt with SDL3 and ImGui.");
  d.addSeparator();
  d.addLabel("Inspired by Microsoft Paint (Windows 95).");

  d.addButton("OK", true, true);

  mgr.open(d);
}

// ─────────────────────────────────────────────────────────────────────────────
//  File → Open (path input when file dialog is not yet implemented)
// ─────────────────────────────────────────────────────────────────────────────

inline void openFileOpen(
    DialogManager &mgr,
    std::function<void(const std::string &path, bool accepted)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Open");
  d.setWidth(320.f);

  d.addLabel("File path:");
  auto *pathCtrl = d.addTextField("", "", 0.f, 280.f);

  d.addButton("Open", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    callback(pathCtrl->getValue(), r.buttonLabel == "Open");
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  File → Save As (path input)
// ─────────────────────────────────────────────────────────────────────────────

inline void openFileSaveAs(
    DialogManager &mgr, const std::string &currentPath,
    std::function<void(const std::string &path, bool accepted)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Save As");
  d.setWidth(320.f);

  d.addLabel("Save file as:");
  auto *pathCtrl = d.addTextField("", currentPath, 0.f, 280.f);

  d.addButton("Save", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    callback(pathCtrl->getValue(), r.buttonLabel == "Save");
  });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Canvas resize (used by File → New and Ctrl+resize)
// ─────────────────────────────────────────────────────────────────────────────

struct ResizeResult {
  int width = 800;
  int height = 500;
  bool accepted = false;
};

inline void
openResizeCanvas(DialogManager &mgr, int currentW, int currentH,
                 std::function<void(const ResizeResult &)> callback) {
  Dialog &d = mgr.create();
  d.setTitle("Resize Canvas");
  d.setWidth(260.f);

  auto *wCtrl = d.addIntField("Width:", currentW, "px", 60.f, 80.f);
  auto *hCtrl = d.addIntField("Height:", currentH, "px", 60.f, 80.f);

  d.addButton("OK", true, false);
  d.addButton("Cancel", false, true);

  mgr.open(d, [=](const DialogResult &r) {
    ResizeResult rr;
    rr.width = wCtrl->getValue();
    rr.height = hCtrl->getValue();
    rr.accepted = (r.buttonLabel == "OK");
    callback(rr);
  });
}

} // namespace UI::AppDialogs
