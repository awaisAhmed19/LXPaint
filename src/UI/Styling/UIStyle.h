#pragma once

#include <imgui.h>

namespace UIStyle {
inline constexpr float RibbonButtonWidth = 45.0f;
inline constexpr float RibbonPaddingY = 4.0f;

inline constexpr float GroupSpacing = 6.0f;
inline constexpr float GroupSeparatorW = 1.0f;
inline constexpr float ItemSpacing = 2.0f;

inline constexpr float PanelPaddingX = 6.0f;
inline constexpr float PanelPaddingY = 4.0f;

inline constexpr float ButtonHeight = 0.0f;
inline constexpr float ToggleButtonWidth = 32.0f;
inline constexpr float ToggleButtonHeight = 32.0f;
inline constexpr float IconButtonSize = 0.0f;

inline constexpr float LabelSpacingAfter = 4.0f;

inline constexpr float SeparatorMarginX = 3.0f;
inline constexpr float SeparatorMarginY = 2.0f;

inline ImU32 colorBorder() { return IM_COL32(0, 0, 0, 255); }
inline ImU32 colorHighlight() { return IM_COL32(255, 255, 255, 255); }
inline ImU32 colorSurface() { return IM_COL32(192, 192, 192, 255); }
inline ImU32 colorSunken() { return IM_COL32(128, 128, 128, 255); }
inline ImU32 colorText() { return IM_COL32(0, 0, 0, 255); }
inline ImU32 colorToggleOn() { return IM_COL32(0, 0, 128, 255); }
}; // namespace UIStyle
