#pragma once
#include "UI/Styling/UIDrawHelper.h"
#include "UI/Styling/UIStyle.h"
#include <imgui.h>
#include <string>

namespace UI {

struct Button {
  const char *label = "";
  ImVec2 size = {0.f, 0.f}; // (0,0) = size to label

  // Returns true on click.
  // Call after ImGui has reserved the item rect (via InvisibleButton).
  bool draw(ImDrawList *draw) const {
    ImVec2 sz = size;
    if (sz.x <= 0.f || sz.y <= 0.f) {
      ImVec2 ts = ImGui::CalcTextSize(label);
      if (sz.x <= 0.f)
        sz.x = ts.x + UIStyle::PanelPaddingX * 2.f;
      if (sz.y <= 0.f)
        sz.y = ts.y + UIStyle::RibbonPaddingY * 2.f;
    }

    std::string id = std::string(label) + "##btn";
    bool pressed = ImGui::InvisibleButton(id.c_str(), sz);
    bool active = ImGui::IsItemActive();
    bool hovered = ImGui::IsItemHovered();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    // Fill
    ImU32 fill = UIStyle::colorSurface();
    draw->AddRectFilled(min, max, fill);

    // Border
    if (active)
      UIDrawHelpers::drawSunkenBorder(draw, min, max);
    else if (hovered)
      UIDrawHelpers::drawRaisedBorder(draw, min, max);

    // Label — shift by 1px when pressed
    ImVec2 ts = ImGui::CalcTextSize(label);
    float ox = active ? 1.f : 0.f;
    ImVec2 tp = {min.x + (max.x - min.x - ts.x) * .5f + ox,
                 min.y + (max.y - min.y - ts.y) * .5f + ox};
    draw->AddText(tp, UIStyle::colorText(), label);

    return pressed;
  }
};

struct ToggleButton {
  const char *label = "";
  bool *toggled = nullptr; // must not be null
  ImVec2 size = {UIStyle::ToggleButtonWidth, UIStyle::ToggleButtonHeight};

  // Returns true when the value changes.
  bool draw(ImDrawList *draw) const {
    std::string id = std::string(label) + "##tgl";
    bool pressed = ImGui::InvisibleButton(id.c_str(), size);

    if (pressed && toggled)
      *toggled = !*toggled;

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    bool on = toggled && *toggled;

    // Fill — darker when on
    ImU32 fill = on ? UIStyle::colorToggleOn() : UIStyle::colorSurface();
    draw->AddRectFilled(min, max, fill);

    // Border
    if (on)
      UIDrawHelpers::drawSunkenBorder(draw, min, max);
    else
      UIDrawHelpers::drawRaisedBorder(draw, min, max);

    // Label
    ImVec2 ts = ImGui::CalcTextSize(label);
    ImVec2 tp = {min.x + (size.x - ts.x) * .5f, min.y + (size.y - ts.y) * .5f};
    ImU32 textColor = on ? UIStyle::colorHighlight() : UIStyle::colorText();
    draw->AddText(tp, textColor, label);

    return pressed;
  }
};
struct Label {
  const char *text = "";
  ImU32 color = 0;

  void draw(ImDrawList *draw, ImVec2 pos) const {
    ImU32 c = (color != 0) ? color : UIStyle::colorText();
    draw->AddText(pos, c, text);
  }

  ImVec2 size() const { return ImGui::CalcTextSize(text); }
};

struct Separator {
  float width = UIStyle::SeparatorMarginX * 2.f + 2.f;

  void draw(ImDrawList *draw, ImVec2 pos, float height) const {
    float cx = pos.x + width * .5f;
    UIDrawHelpers::drawVerticalSeparator(draw, cx, pos.y, pos.y + height);
  }
};

struct Spacer {
  float size = UIStyle::GroupSpacing;
};
}; // namespace UI
