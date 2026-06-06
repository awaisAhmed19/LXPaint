#pragma once
#include "UI/Styling/UIDrawHelper.h"
#include "UI/Styling/UIStyle.h"
#include <cstdint>
#include <functional>
#include <imgui.h>
#include <string>

namespace UI {

// ─────────────────────────────────────────────────────────────
// BUTTON COMPONENTS
// ─────────────────────────────────────────────────────────────

/**
 * Button: Standard clickable button with 3D border effect
 * - Auto-sizes to label if size is not specified
 * - Responds to hover and active states with visual feedback
 * - Supports custom sizing
 */
struct Button {
  const char *label = "";
  ImVec2 size = {0.f, 0.f}; // (0,0) = auto-size to label
  ImU32 fillColor = 0;      // 0 = use default colorSurface()
  bool enabled = true;

  /**
   * Draw the button and return true if clicked
   * Must be called after ImGui has reserved the item rect
   */
  bool draw(ImDrawList *draw) const {
    ImVec2 sz = calculateSize();

    std::string id = std::string(label) + "##btn_" +
                     std::to_string(reinterpret_cast<std::uintptr_t>(this));

    // Create invisible button for input
    bool pressed = ImGui::InvisibleButton(id.c_str(), sz);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    // Determine appearance based on state
    ImU32 fill = fillColor != 0 ? fillColor : UIStyle::colorSurface();
    if (!enabled) {
      fill = UIStyle::colorSurfaceDark();
    }

    // Draw background
    draw->AddRectFilled(min, max, fill);

    // Draw border based on state
    if (!enabled) {
      UIDrawHelpers::drawFlatBorder(draw, min, max, UIStyle::colorBorder());
    } else if (active) {
      UIDrawHelpers::drawSunkenBorder(draw, min, max);
    } else if (hovered) {
      UIDrawHelpers::drawRaisedBorder(draw, min, max);
    }

    // Draw text (centered, with press offset)
    drawButtonText(draw, min, max, active);

    return pressed && enabled;
  }

private:
  ImVec2 calculateSize() const {
    ImVec2 sz = size;
    if (sz.x <= 0.f || sz.y <= 0.f) {
      ImVec2 textSize = ImGui::CalcTextSize(label);
      if (sz.x <= 0.f)
        sz.x = textSize.x + UIStyle::PaddingX * 2.f;
      if (sz.y <= 0.f)
        sz.y = textSize.y + UIStyle::PaddingY * 2.f;
    }
    return sz;
  }

  void drawButtonText(ImDrawList *draw, ImVec2 min, ImVec2 max,
                      bool active) const {
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 buttonSize = {max.x - min.x, max.y - min.y};

    // Center text and apply 1px offset when pressed
    float pressOffset = active ? 1.f : 0.f;
    ImVec2 textPos = {min.x + (buttonSize.x - textSize.x) * 0.5f + pressOffset,
                      min.y + (buttonSize.y - textSize.y) * 0.5f + pressOffset};

    ImU32 textColor =
        enabled ? UIStyle::colorText() : UIStyle::colorTextDisabled();
    draw->AddText(textPos, textColor, label);
  }
};

/**
 * ToggleButton: Button that can be toggled on/off
 * - Shows pressed state when toggled
 * - Visual feedback on hover and active states
 * - Maintains internal toggled state
 */
struct ToggleButton {
  const char *label = "";
  bool *toggled = nullptr; // must not be null
  ImVec2 size = {UIStyle::ToggleButtonWidth, UIStyle::ToggleButtonHeight};
  std::function<void(bool)> onChange; // callback when state changes
  bool enabled = true;

  /**
   * Draw the toggle button and return true when state changes
   */
  bool draw(ImDrawList *draw) const {
    if (!toggled)
      return false;

    // Create unique ID
    std::string id = std::string(label) + "##tgl_" +
                     std::to_string(reinterpret_cast<uintptr_t>(this));
    bool pressed = ImGui::InvisibleButton(id.c_str(), size);
    bool hovered = ImGui::IsItemHovered();

    bool stateChanged = false;
    if (pressed && enabled) {
      *toggled = !*toggled;
      stateChanged = true;
      if (onChange)
        onChange(*toggled);
    }

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    bool isOn = toggled && *toggled;

    // Draw background (darker when on)
    ImU32 fill = isOn ? UIStyle::colorToggleOn() : UIStyle::colorSurface();
    if (!enabled)
      fill = UIStyle::colorSurfaceDark();

    draw->AddRectFilled(min, max, fill);

    // Draw border
    if (!enabled) {
      UIDrawHelpers::drawFlatBorder(draw, min, max, UIStyle::colorBorder());
    } else if (isOn) {
      UIDrawHelpers::drawSunkenBorder(draw, min, max);
    } else {
      UIDrawHelpers::drawRaisedBorder(draw, min, max);
    }

    // Draw label (centered)
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 buttonSize = {max.x - min.x, max.y - min.y};
    ImVec2 textPos = {min.x + (buttonSize.x - textSize.x) * 0.5f,
                      min.y + (buttonSize.y - textSize.y) * 0.5f};

    ImU32 textColor = isOn ? UIStyle::colorHighlight() : UIStyle::colorText();
    if (!enabled)
      textColor = UIStyle::colorTextDisabled();

    draw->AddText(textPos, textColor, label);

    return stateChanged;
  }
};

/**
 * IconButton: Button with an icon and optional label
 * - Useful for toolbar buttons with visual indicators
 * - Supports custom icon drawing
 */
struct IconButton {
  const char *label = "";
  ImVec2 size = {UIStyle::IconButtonSize, UIStyle::IconButtonSize};
  std::function<void(ImDrawList *, ImVec2, ImVec2, ImU32)>
      iconDrawer; // callback to draw icon
  ImU32 iconColor = UIStyle::colorText();
  bool enabled = true;

  /**
   * Draw icon button with custom icon
   */
  bool draw(ImDrawList *draw) const {
    if (!iconDrawer)
      return false;

    std::string id = std::string(label) + "##ico_" +
                     std::to_string(reinterpret_cast<uintptr_t>(this));
    bool pressed = ImGui::InvisibleButton(id.c_str(), size);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    // Draw background
    draw->AddRectFilled(min, max, UIStyle::colorSurface());

    // Draw border
    if (active) {
      UIDrawHelpers::drawSunkenBorder(draw, min, max);
    } else if (hovered) {
      UIDrawHelpers::drawRaisedBorder(draw, min, max);
    }

    // Draw custom icon
    ImU32 color = enabled ? iconColor : UIStyle::colorTextDisabled();
    ImVec2 iconMin = {min.x + UIStyle::IconButtonPadding,
                      min.y + UIStyle::IconButtonPadding};
    ImVec2 iconMax = {max.x - UIStyle::IconButtonPadding,
                      max.y - UIStyle::IconButtonPadding};
    iconDrawer(draw, iconMin, iconMax, color);

    return pressed && enabled;
  }
};

/**
 * Label: Simple text display with optional custom color
 */
struct Label {
  const char *text = "";
  ImU32 color = 0; // 0 = use default colorText()
  bool enabled = true;

  void draw(ImDrawList *draw, ImVec2 pos) const {
    ImU32 col = (color != 0) ? color
                             : (enabled ? UIStyle::colorText()
                                        : UIStyle::colorTextDisabled());
    draw->AddText(pos, col, text);
  }

  ImVec2 size() const { return ImGui::CalcTextSize(text); }
};

/**
 * TextInput: Text input field (wrapper around ImGui input)
 * Note: This requires ImGui context, not just ImDrawList
 */
struct TextInput {
  const char *label = "";
  char *buffer = nullptr;
  size_t bufferSize = 0;
  std::function<void()> onChange;
  bool enabled = true;

  bool draw() const {
    if (!buffer || bufferSize == 0)
      return false;

    ImGui::BeginDisabled(!enabled);
    bool changed = ImGui::InputText(label, buffer, bufferSize);
    ImGui::EndDisabled();

    if (changed && onChange)
      onChange();

    return changed;
  }
};
/**
 * Separator: Visual divider line (vertical or horizontal)
 * Uses 3D groove effect for visual depth
 */
struct Separator {
  enum class Direction { Vertical, Horizontal };

  Direction direction = Direction::Vertical;
  float length = 20.f; // height for vertical, width for horizontal
  ImU32 color = 0;     // 0 = use default

  void draw(ImDrawList *draw, ImVec2 pos) const {
    ImU32 col = (color != 0) ? color : UIStyle::colorBorder();

    if (direction == Direction::Vertical) {
      UIDrawHelpers::drawVerticalSeparator(draw, pos.x, pos.y, pos.y + length);
    } else {
      UIDrawHelpers::drawHorizontalSeparator(draw, pos.y, pos.x,
                                             pos.x + length);
    }
  }

  ImVec2 size() const {
    if (direction == Direction::Vertical) {
      return {UIStyle::SeparatorWidth + 2.f, length};
    }
    return {length, UIStyle::SeparatorWidth + 2.f};
  }
};

/**
 * Spacer: Invisible spacing element
 * Used for layout padding and alignment
 */
struct Spacer {
  float sizeX = UIStyle::GroupSpacingX;
  float sizeY = UIStyle::GroupSpacingY;

  ImVec2 size() const { return {sizeX, sizeY}; }

  // Static helpers for common spacings
  static Spacer group() {
    return {UIStyle::GroupSpacingX, UIStyle::GroupSpacingY};
  }

  static Spacer item() {
    return {UIStyle::ItemSpacingX, UIStyle::ItemSpacingY};
  }

  static Spacer custom(float x, float y) { return {x, y}; }
};

/**
 * Divider: Visual separator with optional label
 * Useful for section headers and grouping
 */
struct Divider {
  const char *label = nullptr; // optional label in the middle
  ImU32 lineColor = 0;         // 0 = use default
  ImU32 textColor = 0;         // 0 = use default

  void draw(ImDrawList *draw, ImVec2 min, ImVec2 max) const {
    ImU32 lineCol = (lineColor != 0) ? lineColor : UIStyle::colorBorder();
    ImU32 textCol = (textColor != 0) ? textColor : UIStyle::colorText();

    if (!label || label[0] == '\0') {
      // Simple line
      UIDrawHelpers::drawHorizontalLine(draw, (min.y + max.y) * 0.5f, min.x,
                                        max.x, lineCol);
    } else {
      // Line with centered label
      ImVec2 textSize = ImGui::CalcTextSize(label);
      float centerX = (min.x + max.x) * 0.5f;
      float centerY = (min.y + max.y) * 0.5f;

      // Left line
      UIDrawHelpers::drawHorizontalLine(
          draw, centerY, min.x, centerX - textSize.x * 0.5f - 8.f, lineCol);
      // Right line
      UIDrawHelpers::drawHorizontalLine(
          draw, centerY, centerX + textSize.x * 0.5f + 8.f, max.x, lineCol);
      // Text
      ImVec2 textPos = {centerX - textSize.x * 0.5f,
                        centerY - textSize.y * 0.5f};
      draw->AddText(textPos, textCol, label);
    }
  }
};

/**
 * ButtonGroup: Multiple buttons displayed horizontally or vertically
 * Useful for button bars and option groups
 */
struct ButtonGroup {
  enum class Layout { Horizontal, Vertical };

  const char **labels = nullptr;
  int labelCount = 0;
  int selectedIndex = -1;
  Layout layout = Layout::Horizontal;
  ImVec2 buttonSize = {50.f, 20.f};
  std::function<void(int)> onSelectionChanged;

  int draw(ImDrawList *draw) const {
    int newSelection = selectedIndex;

    for (int i = 0; i < labelCount; ++i) {
      Button btn{labels[i], buttonSize};
      btn.fillColor = (i == selectedIndex) ? UIStyle::colorToggleOn() : 0;

      if (btn.draw(draw) && newSelection != i) {
        newSelection = i;
        if (onSelectionChanged)
          onSelectionChanged(newSelection);
      }

      // Add spacing between buttons
      if (layout == Layout::Horizontal && i < labelCount - 1) {
        ImGui::Spacing();
      }
    }

    return newSelection;
  }
};

} // namespace UI
