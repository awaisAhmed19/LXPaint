#pragma once
#include "DialogControl.h"
#include "DialogTheme.h"
#include "imgui.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace UI {

struct DialogResult {
  int buttonIndex = -1;    // which button was pressed (-1 = none yet)
  std::string buttonLabel; // label of pressed button
  bool closed = false;     // dialog was closed (X or Escape)
};

struct DialogButton {
  std::string label;
  bool isDefault = false;        // drawn with extra border, Enter triggers
  bool isCancel = false;         // Escape triggers
  std::function<void()> onPress; // optional extra callback
};

// ─────────────────────────────────────────────────────────────────────────────
//  Dialog  — a reusable Win95-style modal window
//
//  Lifecycle:
//    1. Construct / configure (setTitle, addLabel, addButton, …)
//    2. Call show() to make it visible
//    3. Every frame, call render(). Returns true if dialog is still open.
//    4. Call getResult() to read the outcome.
//    5. Call hide() or let DialogManager do it.
//
//  DialogManager owns Dialog instances and calls render() at the right time.
// ─────────────────────────────────────────────────────────────────────────────

class Dialog {
public:
  // ── Configuration ────────────────────────────────────────────────────────

  void setTitle(const std::string &title) { m_title = title; }
  void setMessage(const std::string &msg) { addLabel(msg); }

  // Add a static label (multi-line supported via '\n')
  LabelControl *addLabel(const std::string &text) {
    return addControl<LabelControl>(text);
  }

  SeparatorControl *addSeparator() { return addControl<SeparatorControl>(); }

  TextFieldControl *addTextField(const std::string &label,
                                 const std::string &initial = "",
                                 float labelWidth = 60.f,
                                 float fieldWidth = 80.f) {
    return addControl<TextFieldControl>(label, initial, labelWidth, fieldWidth);
  }

  IntFieldControl *addIntField(const std::string &label, int initial,
                               const std::string &suffix = "",
                               float labelWidth = 80.f,
                               float fieldWidth = 50.f) {
    return addControl<IntFieldControl>(label, initial, suffix, labelWidth,
                                       fieldWidth);
  }

  FloatFieldControl *addFloatField(const std::string &label, float initial,
                                   const std::string &suffix = "",
                                   float labelWidth = 80.f,
                                   float fieldWidth = 50.f) {
    return addControl<FloatFieldControl>(label, initial, suffix, labelWidth,
                                         fieldWidth);
  }

  CheckboxControl *addCheckbox(const std::string &label, bool initial = false) {
    return addControl<CheckboxControl>(label, initial);
  }

  RadioGroupControl *
  addRadioGroup(std::vector<RadioGroupControl::Option> options, int initial = 0,
                float indent = 0.f) {
    return addControl<RadioGroupControl>(std::move(options), initial, indent);
  }

  GroupBoxControl *addGroupBox(const std::string &title) {
    return addControl<GroupBoxControl>(title);
  }

  // Add a button to the right-hand column (or bottom row).
  // Returns index of the button (0-based).
  int addButton(const std::string &label, bool isDefault = false,
                bool isCancel = false,
                std::function<void()> onPress = nullptr) {
    m_buttons.push_back({label, isDefault, isCancel, std::move(onPress)});
    return (int)m_buttons.size() - 1;
  }

  // Override dialog width (default: auto-sized to ~300 px)
  void setWidth(float w) { m_overrideWidth = w; }

  // ── Lifecycle ─────────────────────────────────────────────────────────────

  void show() {
    m_open = true;
    m_result = DialogResult{};
    m_dragging = false;
    // Centre on viewport when shown
    ImGuiIO &io = ImGui::GetIO();
    m_pos = {io.DisplaySize.x * 0.5f - computeWidth() * 0.5f,
             io.DisplaySize.y * 0.5f - computeTotalHeight() * 0.5f};
  }

  void hide() { m_open = false; }
  bool isOpen() const { return m_open; }

  const DialogResult &getResult() const { return m_result; }

  // ── Rendering ─────────────────────────────────────────────────────────────

  // Returns true while still open, false when it has closed.
  bool render() {
    if (!m_open)
      return false;

    ImDrawList *dl = ImGui::GetForegroundDrawList();

    float w = computeWidth();
    float h = computeTotalHeight();
    float titleH = DialogTheme::TitleBarH;
    float pad = DialogTheme::Padding;

    ImVec2 winMin = m_pos;
    ImVec2 winMax = {m_pos.x + w, m_pos.y + h};
    ImVec2 titleMax = {winMax.x, winMin.y + titleH};

    // ── Draw outer frame ──────────────────────────────────────────────────
    dl->AddRectFilled(winMin, winMax, DialogTheme::BG);
    DialogTheme::drawRaised(dl, winMin, winMax);

    // ── Title bar ─────────────────────────────────────────────────────────
    dl->AddRectFilled(winMin, titleMax, DialogTheme::TitleBarBg);

    // Title text
    ImVec2 titleTextPos = {winMin.x + 4.f,
                           winMin.y +
                               (titleH - ImGui::GetTextLineHeight()) * 0.5f};
    dl->AddText(titleTextPos, DialogTheme::TitleText, m_title.c_str());

    // Close button [X]
    float closeSize = titleH - 4.f;
    ImVec2 closeMin = {winMax.x - closeSize - 2.f, winMin.y + 2.f};
    ImVec2 closeMax = {closeMin.x + closeSize, closeMin.y + closeSize};
    dl->AddRectFilled(closeMin, closeMax, DialogTheme::BG);
    DialogTheme::drawRaised(dl, closeMin, closeMax);
    float crossPad = 3.f;
    dl->AddLine({closeMin.x + crossPad, closeMin.y + crossPad},
                {closeMax.x - crossPad, closeMax.y - crossPad},
                DialogTheme::TextNormal, 1.5f);
    dl->AddLine({closeMax.x - crossPad, closeMin.y + crossPad},
                {closeMin.x + crossPad, closeMax.y - crossPad},
                DialogTheme::TextNormal, 1.5f);

    // ── Handle dragging the title bar ─────────────────────────────────────
    {
      ImGui::SetCursorScreenPos(winMin);
      ImGui::PushID("dlg_title");
      ImGui::InvisibleButton("##title", {w - closeSize - 4.f, titleH});
      if (ImGui::IsItemActive()) {
        if (!m_dragging) {
          m_dragging = true;
          m_dragStart = ImGui::GetMousePos();
        }
        ImVec2 mp = ImGui::GetMousePos();
        m_pos.x += mp.x - m_dragStart.x;
        m_pos.y += mp.y - m_dragStart.y;
        m_dragStart = mp;
      } else {
        m_dragging = false;
      }
      ImGui::PopID();

      // Close button click
      ImGui::SetCursorScreenPos(closeMin);
      ImGui::PushID("dlg_close");
      if (ImGui::InvisibleButton("##close", {closeSize, closeSize})) {
        m_result.closed = true;
        m_result.buttonIndex = -1;
        hide();
        ImGui::PopID();
        return false;
      }
      ImGui::PopID();
    }

    // ── Controls area ─────────────────────────────────────────────────────
    float controlsLeft = winMin.x + pad;
    float controlsRight = winMax.x - pad - computeButtonColumnWidth() -
                          (m_buttons.empty() ? 0.f : DialogTheme::ButtonGap);
    float controlsWidth = controlsRight - controlsLeft;
    float cy = winMin.y + titleH + pad;

    for (auto &ctrl : m_controls) {
      ctrl->render(dl, {controlsLeft, cy}, controlsWidth);
      cy += ctrl->preferredHeight();
    }

    // ── Button column (right side) ─────────────────────────────────────────
    if (!m_buttons.empty()) {
      float bx = winMax.x - pad - DialogTheme::ButtonW;
      float by = winMin.y + titleH + pad;

      for (int i = 0; i < (int)m_buttons.size(); ++i) {
        auto &btn = m_buttons[i];
        bool pressed = renderButton(dl, btn, {bx, by}, i);
        if (pressed) {
          m_result.buttonIndex = i;
          m_result.buttonLabel = btn.label;
          if (btn.onPress)
            btn.onPress();
          hide();
          return false;
        }
        by += DialogTheme::ButtonH + DialogTheme::ButtonGap;
      }
    }

    // ── Keyboard ──────────────────────────────────────────────────────────
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
      // Find cancel button or just close
      for (int i = 0; i < (int)m_buttons.size(); ++i) {
        if (m_buttons[i].isCancel) {
          m_result.buttonIndex = i;
          m_result.buttonLabel = m_buttons[i].label;
          if (m_buttons[i].onPress)
            m_buttons[i].onPress();
          hide();
          return false;
        }
      }
      m_result.closed = true;
      hide();
      return false;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Enter) ||
        ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
      for (int i = 0; i < (int)m_buttons.size(); ++i) {
        if (m_buttons[i].isDefault) {
          m_result.buttonIndex = i;
          m_result.buttonLabel = m_buttons[i].label;
          if (m_buttons[i].onPress)
            m_buttons[i].onPress();
          hide();
          return false;
        }
      }
    }

    return true; // still open
  }

private:
  // ── Internal helpers ──────────────────────────────────────────────────────

  template <typename T, typename... Args> T *addControl(Args &&...args) {
    auto ctrl = std::make_unique<T>(std::forward<Args>(args)...);
    T *raw = ctrl.get();
    m_controls.push_back(std::move(ctrl));
    return raw;
  }

  float computeButtonColumnWidth() const {
    return m_buttons.empty() ? 0.f : DialogTheme::ButtonW;
  }

  float computeWidth() const {
    if (m_overrideWidth > 0.f)
      return m_overrideWidth;
    return 300.f; // default
  }

  float computeControlsHeight() const {
    float h = 0.f;
    for (auto &ctrl : m_controls)
      h += ctrl->preferredHeight();
    return h;
  }

  float computeButtonColumnHeight() const {
    if (m_buttons.empty())
      return 0.f;
    return m_buttons.size() * DialogTheme::ButtonH +
           (m_buttons.size() - 1) * DialogTheme::ButtonGap;
  }

  float computeTotalHeight() const {
    float pad = DialogTheme::Padding;
    float titleH = DialogTheme::TitleBarH;
    float ctrlH = computeControlsHeight();
    float btnH = computeButtonColumnHeight();
    return titleH + pad * 2.f + std::max(ctrlH, btnH);
  }

  // Draw one button, return true if clicked
  bool renderButton(ImDrawList *dl, const DialogButton &btn, ImVec2 pos,
                    int idx) {
    using namespace DialogTheme;

    ImVec2 bMin = pos;
    ImVec2 bMax = {pos.x + ButtonW, pos.y + ButtonH};

    ImGui::SetCursorScreenPos(bMin);
    ImGui::PushID(idx + (int)(size_t)this);
    bool clicked = ImGui::InvisibleButton("##btn", {ButtonW, ButtonH});
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    ImGui::PopID();

    // Background
    ImU32 bgCol = active ? ButtonPress : hovered ? ButtonHover : ButtonFace;
    dl->AddRectFilled(bMin, bMax, bgCol);

    // Border
    if (active)
      drawSunken(dl, bMin, bMax);
    else
      drawRaised(dl, bMin, bMax);

    // Default button gets an extra outer rectangle
    if (btn.isDefault) {
      dl->AddRect({bMin.x - 2.f, bMin.y - 2.f}, {bMax.x + 2.f, bMax.y + 2.f},
                  BorderOuter, 0.f, 0, 1.f);
    }

    // Label centered
    ImVec2 textSize = ImGui::CalcTextSize(btn.label.c_str());
    ImVec2 textPos = {bMin.x + (ButtonW - textSize.x) * 0.5f,
                      bMin.y + (ButtonH - textSize.y) * 0.5f};
    if (active) {
      textPos.x += 1.f;
      textPos.y += 1.f;
    }
    dl->AddText(textPos, TextNormal, btn.label.c_str());

    return clicked;
  }

  // ── State ─────────────────────────────────────────────────────────────────

  std::string m_title;
  float m_overrideWidth = 0.f;
  bool m_open = false;

  ImVec2 m_pos = {100.f, 100.f};
  bool m_dragging = false;
  ImVec2 m_dragStart = {0.f, 0.f};

  std::vector<std::unique_ptr<DialogControl>> m_controls;
  std::vector<DialogButton> m_buttons;
  DialogResult m_result;
};

} // namespace UI
