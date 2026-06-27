#pragma once
#include "Dialog.h"
#include "DialogTheme.h"
#include "imgui.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
//  DialogManager
//
//  Owns dialog instances and enforces modal behavior.
//
//  Only one dialog may be active at a time. While a dialog is open:
//    • isModal() returns true
//    • Editor / Ribbon / Toolbar should call isModal() before handling
//      any mouse or keyboard input and skip their own handling if true.
//
//  Typical usage in Application::render():
//
//    // ── After all normal UI ──
//    m_dialogManager.render();
//
//  Typical usage in Application::handleEvents():
//
//    if (!m_dialogManager.isModal())
//        m_editor->handleEvent(m_event);
//
//  Creating a dialog:
//
//    auto& dialog = m_dialogManager.create();
//    dialog.setTitle("Save Changes");
//    dialog.setMessage("Do you want to save?");
//    dialog.addButton("Save",      true,  false);
//    dialog.addButton("Don't Save",false, false);
//    dialog.addButton("Cancel",    false, true );
//    m_dialogManager.open(dialog, [this](const DialogResult& r) {
//        if (r.buttonLabel == "Save") saveDocument();
//    });
//
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

class DialogManager {
public:
  // ── Factory ───────────────────────────────────────────────────────────────

  // Create a new Dialog owned by the manager.  Configure it and then call
  // open() to display it.  The returned reference is stable until the
  // manager is destroyed.
  Dialog &create() {
    m_pool.push_back(std::make_unique<Dialog>());
    return *m_pool.back();
  }

  // Show `dialog` and register a result callback.
  // The callback fires exactly once when the dialog closes (button press,
  // X button, or Escape).  DialogManager remains the owner.
  void open(Dialog &dialog,
            std::function<void(const DialogResult &)> callback = nullptr) {
    dialog.show();
    m_active = &dialog;
    m_callback = std::move(callback);
  }

  // ── Convenience builders ──────────────────────────────────────────────────

  // Simple yes/no/cancel prompt
  Dialog &
  openPrompt(const std::string &title, const std::string &message,
             std::vector<std::string> buttonLabels,
             std::function<void(const DialogResult &)> callback = nullptr) {
    Dialog &d = create();
    d.setTitle(title);
    d.addLabel(message);
    bool first = true;
    for (auto &lbl : buttonLabels) {
      bool isCancel = (lbl == "Cancel" || lbl == "No");
      d.addButton(lbl, first /*default*/, isCancel);
      first = false;
    }
    d.setWidth(300.f);
    open(d, std::move(callback));
    return d;
  }

  // "About" box — message + single OK button
  Dialog &openAbout(const std::string &title, const std::string &message,
                    std::function<void()> onOk = nullptr) {
    Dialog &d = create();
    d.setTitle(title);
    d.addLabel(message);
    d.addButton("OK", true, true, std::move(onOk));
    d.setWidth(340.f);
    open(d);
    return d;
  }

  // ── Lifecycle ─────────────────────────────────────────────────────────────

  // Close any currently open dialog without triggering a result.
  void closeAll() {
    if (m_active) {
      m_active->hide();
      m_active = nullptr;
      m_callback = nullptr;
    }
  }

  bool isModal() const { return m_active && m_active->isOpen(); }

  // ── Rendering — call once per frame, AFTER all other UI ──────────────────
  void render() {
    if (!m_active || !m_active->isOpen()) {
      m_active = nullptr;
      m_callback = nullptr;
      return;
    }

    // ── Modal dim overlay ─────────────────────────────────────────────────
    ImDrawList *dl = ImGui::GetForegroundDrawList();
    ImGuiIO &io = ImGui::GetIO();
    dl->AddRectFilled({0.f, 0.f}, io.DisplaySize, DialogTheme::Overlay);

    // Block all ImGui input from reaching things behind the dialog.
    // We manually consume mouse clicks that land outside the dialog so
    // the background doesn't react.
    ImGui::SetNextFrameWantCaptureMouse(true);
    ImGui::SetNextFrameWantCaptureKeyboard(true);

    // ── Render dialog ─────────────────────────────────────────────────────
    bool stillOpen = m_active->render();

    if (!stillOpen) {
      // Dialog just closed this frame — fire callback
      if (m_callback) {
        m_callback(m_active->getResult());
      }
      m_active = nullptr;
      m_callback = nullptr;
    }
  }

private:
  std::vector<std::unique_ptr<Dialog>> m_pool;
  Dialog *m_active = nullptr;
  std::function<void(const DialogResult &)> m_callback;
};

} // namespace UI
