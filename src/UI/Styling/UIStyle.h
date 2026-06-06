#pragma once

#include <imgui.h>

namespace UIStyle {
// Padding: Internal spacing within containers
inline constexpr float PaddingX = 6.0f;
inline constexpr float PaddingY = 4.0f;

// Margins: External spacing between elements
inline constexpr float ItemSpacingX = 2.0f;
inline constexpr float ItemSpacingY = 2.0f;

// Group spacing: Larger gaps between logical groups
inline constexpr float GroupSpacingX = 6.0f;
inline constexpr float GroupSpacingY = 6.0f;

// Separator margins: Padding around separator lines
inline constexpr float SeparatorMarginX = 3.0f;
inline constexpr float SeparatorMarginY = 2.0f;

// Legacy aliases for backwards compatibility
inline constexpr float PanelPaddingX = PaddingX;
inline constexpr float PanelPaddingY = PaddingY;
inline constexpr float ItemSpacing = ItemSpacingX;
inline constexpr float GroupSpacing = GroupSpacingX;

inline constexpr float RibbonButtonWidth = 45.0f;
inline constexpr float RibbonPaddingY = 4.0f;

inline constexpr float ButtonHeight =
    20.0f; // Changed from 0.0f (auto-calculate)
inline constexpr float ButtonMinWidth = 40.0f;

inline constexpr float ToggleButtonWidth = 32.0f;
inline constexpr float ToggleButtonHeight = 32.0f;

inline constexpr float IconButtonSize = 24.0f; // Changed from 0.0f (was unused)
inline constexpr float IconButtonPadding = 4.0f;

inline constexpr float LabelSpacingAfter = 4.0f;

inline constexpr float SeparatorWidth = 1.0f;
inline constexpr float SeparatorHeight = 20.0f;

// Legacy alias for backwards compatibility
inline constexpr float GroupSeparatorW = SeparatorWidth;

// Border and edge colors
inline ImU32 colorBorder() { return IM_COL32(0, 0, 0, 255); }          // Black
inline ImU32 colorHighlight() { return IM_COL32(255, 255, 255, 255); } // White

// Surface and background colors
inline ImU32 colorSurface() {
  return IM_COL32(192, 192, 192, 255);
} // Light gray
inline ImU32 colorSurfaceDark() {
  return IM_COL32(128, 128, 128, 255);
} // Medium gray
inline ImU32 colorSunken() { return colorSurfaceDark(); } // Alias

// Text colors
inline ImU32 colorText() { return IM_COL32(0, 0, 0, 255); } // Black
inline ImU32 colorTextDisabled() {
  return IM_COL32(128, 128, 128, 255);
} // Gray
inline ImU32 colorTextHint() {
  return IM_COL32(160, 160, 160, 255);
} // Light gray

// Interactive states
inline ImU32 colorToggleOn() { return IM_COL32(0, 0, 128, 255); } // Navy blue
inline ImU32 colorHover() {
  return IM_COL32(220, 220, 220, 255);
} // Very light gray
inline ImU32 colorActive() { return IM_COL32(100, 100, 100, 255); } // Dark gray

// Semantic colors (for future use)
inline ImU32 colorError() { return IM_COL32(192, 0, 0, 255); }     // Dark red
inline ImU32 colorWarning() { return IM_COL32(192, 128, 0, 255); } // Orange
inline ImU32 colorSuccess() { return IM_COL32(0, 128, 0, 255); }   // Green
inline ImU32 colorInfo() { return IM_COL32(0, 128, 192, 255); }    // Cyan

/**
 * Blend two colors
 */
inline ImU32 colorBlend(ImU32 colA, ImU32 colB, float blend) {
  // Unpack ImU32 (RGBA format) to ImVec4
  ImVec4 a = ImGui::ColorConvertU32ToFloat4(colA);
  ImVec4 b = ImGui::ColorConvertU32ToFloat4(colB);

  ImVec4 result = {
      a.x * (1.0f - blend) + b.x * blend,
      a.y * (1.0f - blend) + b.y * blend,
      a.z * (1.0f - blend) + b.z * blend,
      a.w * (1.0f - blend) + b.w * blend,
  };

  return ImGui::GetColorU32(result);
}

/**
 * Create a color with custom alpha
 */
inline ImU32 colorWithAlpha(ImU32 color, float alpha) {
  ImVec4 col = ImGui::ColorConvertU32ToFloat4(color);
  col.w = alpha;
  return ImGui::GetColorU32(col);
}

} // namespace UIStyle
