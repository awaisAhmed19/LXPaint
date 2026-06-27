#pragma once
#include "DialogTheme.h"
#include "imgui.h"
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <vector>
// ─────────────────────────────────────────────────────────────────────────────
//  DialogControl  — abstract base for every widget inside a dialog
//
//  Layout contract:
//    • render(dl, origin, availableWidth) draws the control starting at origin
//    • preferredHeight() tells the dialog how tall this control is
//    • Both are called by Dialog every frame
// ─────────────────────────────────────────────────────────────────────────────

namespace UI {

class DialogControl {
public:
  virtual ~DialogControl() = default;

  // Draw the control.  origin is the top-left corner in screen space.
  // availableWidth is the total interior width the dialog offers.
  virtual void render(ImDrawList *dl, ImVec2 origin, float availableWidth) = 0;

  // Height this control occupies (including bottom margin).
  virtual float preferredHeight() const = 0;

  // True if this control wants to absorb keyboard text input this frame.
  virtual bool wantsTextInput() const { return false; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  LabelControl  — static read-only text (one or multiple lines)
// ─────────────────────────────────────────────────────────────────────────────

class LabelControl : public DialogControl {
public:
  explicit LabelControl(std::string text) : m_text(std::move(text)) {}

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    // Render multi-line text by splitting on '\n'
    float y = origin.y;
    const char *start = m_text.c_str();
    const char *p = start;
    while (true) {
      const char *nl = std::strchr(p, '\n');
      if (!nl) {
        dl->AddText({origin.x, y}, DialogTheme::TextNormal, p);
        break;
      }
      std::string line(p, nl);
      dl->AddText({origin.x, y}, DialogTheme::TextNormal, line.c_str());
      y += ImGui::GetTextLineHeight() + 2.f;
      p = nl + 1;
    }
  }

  float preferredHeight() const override {
    // Count newlines to compute height
    int lines = 1;
    for (char c : m_text)
      if (c == '\n')
        ++lines;
    return lines * (ImGui::GetTextLineHeight() + 2.f) +
           DialogTheme::ControlGapY;
  }

private:
  std::string m_text;
};

// ─────────────────────────────────────────────────────────────────────────────
//  SeparatorControl  — horizontal line separator
// ─────────────────────────────────────────────────────────────────────────────

class SeparatorControl : public DialogControl {
public:
  void render(ImDrawList *dl, ImVec2 origin, float availableWidth) override {
    float y = origin.y + 4.f;
    dl->AddLine({origin.x, y}, {origin.x + availableWidth, y},
                DialogTheme::BorderShadow, 1.f);
    dl->AddLine({origin.x, y + 1.f}, {origin.x + availableWidth, y + 1.f},
                DialogTheme::BorderHi, 1.f);
  }
  float preferredHeight() const override { return 10.f; }
};

// ─────────────────────────────────────────────────────────────────────────────
//  TextFieldControl  — single-line editable text box
// ─────────────────────────────────────────────────────────────────────────────

class TextFieldControl : public DialogControl {
public:
  explicit TextFieldControl(std::string label, std::string initialValue = "",
                            float labelWidth = 60.f, float fieldWidth = 80.f)
      : m_label(std::move(label)), m_labelWidth(labelWidth),
        m_fieldWidth(fieldWidth) {
    m_buf.assign(256, '\0');
    std::strncpy(m_buf.data(), initialValue.c_str(), 255);
  }

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    using namespace DialogTheme;

    float lineH = ImGui::GetTextLineHeight();
    float y = origin.y;

    // Label
    if (!m_label.empty()) {
      dl->AddText({origin.x, y + (FieldH - lineH) * 0.5f}, TextNormal,
                  m_label.c_str());
    }

    // Field background
    float fieldX = origin.x + m_labelWidth;
    ImVec2 fMin = {fieldX, y};
    ImVec2 fMax = {fieldX + m_fieldWidth, y + FieldH};
    dl->AddRectFilled(fMin, fMax, FieldBg);
    drawFieldBorder(dl, fMin, fMax);

    // ImGui invisible input positioned over field
    ImGui::SetCursorScreenPos({fMin.x + 2.f, fMin.y + 2.f});
    ImGui::PushItemWidth(m_fieldWidth - 4.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});

    ImGui::PushID(m_label.c_str());
    if (ImGui::InputText("##tf", m_buf.data(), m_buf.size()))
      m_dirty = true;
    ImGui::PopID();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();
  }

  float preferredHeight() const override {
    return DialogTheme::FieldH + DialogTheme::ControlGapY;
  }

  bool wantsTextInput() const override { return true; }

  std::string getValue() const { return m_buf.data(); }
  void setValue(const std::string &v) {
    std::strncpy(m_buf.data(), v.c_str(), 255);
  }

private:
  std::string m_label;
  float m_labelWidth;
  float m_fieldWidth;
  std::vector<char> m_buf;
  bool m_dirty = false;
};

// ─────────────────────────────────────────────────────────────────────────────
//  IntFieldControl  — labelled integer input with suffix
// ─────────────────────────────────────────────────────────────────────────────

class IntFieldControl : public DialogControl {
public:
  IntFieldControl(std::string label, int initialValue, std::string suffix = "",
                  float labelWidth = 80.f, float fieldWidth = 50.f)
      : m_label(std::move(label)), m_suffix(std::move(suffix)),
        m_labelWidth(labelWidth), m_fieldWidth(fieldWidth),
        m_value(initialValue) {}

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    using namespace DialogTheme;

    float lineH = ImGui::GetTextLineHeight();
    float y = origin.y;

    // Label
    if (!m_label.empty())
      dl->AddText({origin.x, y + (FieldH - lineH) * 0.5f}, TextNormal,
                  m_label.c_str());

    // Field
    float fieldX = origin.x + m_labelWidth;
    ImVec2 fMin = {fieldX, y};
    ImVec2 fMax = {fieldX + m_fieldWidth, y + FieldH};
    dl->AddRectFilled(fMin, fMax, FieldBg);
    drawFieldBorder(dl, fMin, fMax);

    // Input
    ImGui::SetCursorScreenPos({fMin.x + 2.f, fMin.y + 2.f});
    ImGui::PushItemWidth(m_fieldWidth - 4.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
    ImGui::PushID(m_label.c_str());
    ImGui::InputInt("##if", &m_value, 0, 0);
    ImGui::PopID();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();

    // Suffix
    if (!m_suffix.empty()) {
      float suffX = fMax.x + 6.f;
      dl->AddText({suffX, y + (FieldH - lineH) * 0.5f}, TextNormal,
                  m_suffix.c_str());
    }
  }

  float preferredHeight() const override {
    return DialogTheme::FieldH + DialogTheme::ControlGapY;
  }
  bool wantsTextInput() const override { return true; }

  int getValue() const { return m_value; }
  void setValue(int v) { m_value = v; }

private:
  std::string m_label;
  std::string m_suffix;
  float m_labelWidth;
  float m_fieldWidth;
  int m_value;
};

// ─────────────────────────────────────────────────────────────────────────────
//  FloatFieldControl — labelled float input with suffix
// ─────────────────────────────────────────────────────────────────────────────

class FloatFieldControl : public DialogControl {
public:
  FloatFieldControl(std::string label, float initialValue,
                    std::string suffix = "", float labelWidth = 80.f,
                    float fieldWidth = 50.f)
      : m_label(std::move(label)), m_suffix(std::move(suffix)),
        m_labelWidth(labelWidth), m_fieldWidth(fieldWidth),
        m_value(initialValue) {}

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    using namespace DialogTheme;
    float lineH = ImGui::GetTextLineHeight();
    float y = origin.y;

    if (!m_label.empty())
      dl->AddText({origin.x, y + (FieldH - lineH) * 0.5f}, TextNormal,
                  m_label.c_str());

    float fieldX = origin.x + m_labelWidth;
    ImVec2 fMin = {fieldX, y};
    ImVec2 fMax = {fieldX + m_fieldWidth, y + FieldH};
    dl->AddRectFilled(fMin, fMax, FieldBg);
    drawFieldBorder(dl, fMin, fMax);

    ImGui::SetCursorScreenPos({fMin.x + 2.f, fMin.y + 2.f});
    ImGui::PushItemWidth(m_fieldWidth - 4.f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 0});
    ImGui::PushID(m_label.c_str());
    ImGui::InputFloat("##ff", &m_value, 0.f, 0.f, "%.0f");
    ImGui::PopID();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    ImGui::PopItemWidth();

    if (!m_suffix.empty()) {
      dl->AddText({fMax.x + 6.f, y + (FieldH - lineH) * 0.5f}, TextNormal,
                  m_suffix.c_str());
    }
  }

  float preferredHeight() const override {
    return DialogTheme::FieldH + DialogTheme::ControlGapY;
  }
  bool wantsTextInput() const override { return true; }

  float getValue() const { return m_value; }
  void setValue(float v) { m_value = v; }

private:
  std::string m_label;
  std::string m_suffix;
  float m_labelWidth;
  float m_fieldWidth;
  float m_value;
};

// ─────────────────────────────────────────────────────────────────────────────
//  CheckboxControl
// ─────────────────────────────────────────────────────────────────────────────

class CheckboxControl : public DialogControl {
public:
  CheckboxControl(std::string label, bool initialValue = false)
      : m_label(std::move(label)), m_checked(initialValue) {}

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    using namespace DialogTheme;
    float s = CheckSize;
    float lh = ImGui::GetTextLineHeight();
    float cy = origin.y + (std::max(s, lh) - s) * 0.5f;
    float ty = origin.y + (std::max(s, lh) - lh) * 0.5f;

    ImVec2 cMin = {origin.x, cy};
    ImVec2 cMax = {origin.x + s, cy + s};

    dl->AddRectFilled(cMin, cMax, FieldBg);
    drawSunken(dl, cMin, cMax);

    if (m_checked) {
      // Classic Win95 checkmark
      ImVec2 p1 = {cMin.x + 2.f, cMin.y + s * 0.5f};
      ImVec2 p2 = {cMin.x + s * 0.4f, cMax.y - 3.f};
      ImVec2 p3 = {cMax.x - 2.f, cMin.y + 2.f};
      dl->AddLine(p1, p2, RadioDot, 2.f);
      dl->AddLine(p2, p3, RadioDot, 2.f);
    }

    dl->AddText({origin.x + s + 6.f, ty}, TextNormal, m_label.c_str());

    // Click detection
    ImGui::SetCursorScreenPos(cMin);
    ImGui::PushID(m_label.c_str());
    if (ImGui::InvisibleButton(
            "##cb", {s + 6.f + ImGui::CalcTextSize(m_label.c_str()).x,
                     std::max(s, lh)}))
      m_checked = !m_checked;
    ImGui::PopID();
  }

  float preferredHeight() const override {
    return std::max(DialogTheme::CheckSize, ImGui::GetTextLineHeight()) +
           DialogTheme::ControlGapY;
  }

  bool getValue() const { return m_checked; }
  void setValue(bool v) { m_checked = v; }

private:
  std::string m_label;
  bool m_checked;
};

// ─────────────────────────────────────────────────────────────────────────────
//  RadioGroupControl  — a group of mutually exclusive radio buttons
//
//  Usage:
//    auto* rg = dialog.addRadioGroup({"Option A", "Option B", "Option C"}, 0);
//    int selected = rg->getSelected();
// ─────────────────────────────────────────────────────────────────────────────

class RadioGroupControl : public DialogControl {
public:
  struct Option {
    std::string label;
    bool enabled = true;
  };

  RadioGroupControl(std::vector<Option> options, int initialSelection = 0,
                    float indentPx = 0.f)
      : m_options(std::move(options)), m_selected(initialSelection),
        m_indent(indentPx) {}

  void render(ImDrawList *dl, ImVec2 origin,
              float /*availableWidth*/) override {
    using namespace DialogTheme;
    float lh = ImGui::GetTextLineHeight();
    float r = RadioR;
    float rowH = std::max(r * 2.f, lh) + ControlGapY;

    for (int i = 0; i < (int)m_options.size(); ++i) {
      const auto &opt = m_options[i];
      float y = origin.y + i * rowH;
      float cx = origin.x + m_indent + r;
      float cy = y + std::max(r * 2.f, lh) * 0.5f;

      // Circle outline (sunken look: dark top-left, light bottom-right)
      dl->AddCircleFilled({cx, cy}, r, FieldBg);
      dl->AddCircle({cx, cy}, r, BorderShadow, 16, 1.f);

      // Fill dot if selected
      if (m_selected == i)
        dl->AddCircleFilled({cx, cy}, r - 3.f, RadioDot);

      ImU32 col = opt.enabled ? TextNormal : TextDisabled;
      dl->AddText({origin.x + m_indent + r * 2.f + 5.f,
                   y + (std::max(r * 2.f, lh) - lh) * 0.5f},
                  col, opt.label.c_str());

      // Click detection
      if (opt.enabled) {
        ImGui::SetCursorScreenPos({origin.x + m_indent, y});
        float rowW = r * 2.f + 5.f + ImGui::CalcTextSize(opt.label.c_str()).x;
        ImGui::PushID(i + (int)(size_t)this);
        if (ImGui::InvisibleButton("##rb", {rowW, std::max(r * 2.f, lh)}))
          m_selected = i;
        ImGui::PopID();
      }
    }
  }

  float preferredHeight() const override {
    float lh = ImGui::GetTextLineHeight();
    float rowH =
        std::max(DialogTheme::RadioR * 2.f, lh) + DialogTheme::ControlGapY;
    return rowH * (float)m_options.size();
  }

  int getSelected() const { return m_selected; }
  void setSelected(int i) { m_selected = i; }
  void setEnabled(int i, bool v) {
    if (i >= 0 && i < (int)m_options.size())
      m_options[i].enabled = v;
  }

private:
  std::vector<Option> m_options;
  int m_selected;
  float m_indent;
};

// ─────────────────────────────────────────────────────────────────────────────
//  GroupBoxControl  — a group of controls surrounded by a Win95 group frame
//
//  Usage:
//    auto* gb = dialog.addGroupBox("Units");
//    gb->addRadioGroup(...);
//    gb->addCheckbox(...);
// ─────────────────────────────────────────────────────────────────────────────

class GroupBoxControl : public DialogControl {
public:
  explicit GroupBoxControl(std::string title) : m_title(std::move(title)) {}

  // Add a child control inside the group.
  // Returns raw pointer for caller convenience (lifetime owned here).
  template <typename T, typename... Args> T *add(Args &&...args) {
    auto ctrl = std::make_unique<T>(std::forward<Args>(args)...);
    T *raw = ctrl.get();
    m_children.push_back(std::move(ctrl));
    return raw;
  }

  void render(ImDrawList *dl, ImVec2 origin, float availableWidth) override {
    using namespace DialogTheme;

    float labelH = ImGui::GetTextLineHeight();
    float innerPad = 8.f;
    float topInset = labelH * 0.5f;

    ImVec2 boxMin = {origin.x, origin.y + topInset};
    ImVec2 boxMax = {origin.x + availableWidth, origin.y + preferredHeight()};

    dl->AddRectFilled(boxMin, boxMax, BG);
    drawGroupBox(dl, boxMin, boxMax, m_title.c_str());

    float childX = boxMin.x + innerPad;
    float childY = boxMin.y + labelH + innerPad * 0.5f;
    float childW = availableWidth - innerPad * 2.f;

    for (auto &child : m_children) {
      child->render(dl, {childX, childY}, childW);
      childY += child->preferredHeight();
    }
  }

  float preferredHeight() const override {
    float lh = ImGui::GetTextLineHeight();
    float innerPad = 8.f;
    float h = lh + innerPad * 1.5f;
    for (auto &child : m_children)
      h += child->preferredHeight();
    h += innerPad * 0.5f;
    return h + DialogTheme::ControlGapY;
  }

  bool wantsTextInput() const override {
    for (auto &c : m_children)
      if (c->wantsTextInput())
        return true;
    return false;
  }

private:
  std::string m_title;
  std::vector<std::unique_ptr<DialogControl>> m_children;
};

} // namespace UI
