#pragma once
#include "imgui.h"

// ─────────────────────────────────────────────────────────────────────────
//  UI::Theme — single source of truth for every Win95/98 color constant.
//  Every panel (Toolbar, Footer, Ribbon, ColorPalette, dialogs) should
//  reference these instead of declaring its own local `Theme` namespace.
// ─────────────────────────────────────────────────────────────────────────
namespace UI::Theme {

inline constexpr ImU32 Black = IM_COL32(0, 0, 0, 255);
inline constexpr ImU32 White = IM_COL32(255, 255, 255, 255);

inline constexpr ImU32 WindowBg = IM_COL32(192, 192, 192, 255);
inline constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
inline constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
inline constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);

inline constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
inline constexpr ImU32 DisabledText = IM_COL32(128, 128, 128, 255);

inline constexpr ImU32 Highlight = IM_COL32(0, 0, 128, 255); // navy select bar
inline constexpr ImU32 HighlightText = IM_COL32(255, 255, 255, 255);

inline constexpr ImU32 SeparatorLo = IM_COL32(128, 128, 128, 255);
inline constexpr ImU32 SeparatorHi = IM_COL32(255, 255, 255, 255);

inline constexpr ImU32 BorderHi = IM_COL32(255, 255, 255, 255);
inline constexpr ImU32 BorderLo = IM_COL32(64, 64, 64, 255);
inline constexpr ImU32 BorderOuter = IM_COL32(0, 0, 0, 255);

inline constexpr ImU32 OptionHovered = IM_COL32(0, 0, 50, 255);

// Default spacing — most retro panels share these; override per-panel only
// when there's a real reason to.
inline constexpr ImVec2 DefaultWindowPadding{0.f, 0.f};
inline constexpr ImVec2 DefaultItemSpacing{1.f, 1.f};
inline constexpr ImVec2 DefaultFramePadding{1.f, 1.f};

} // namespace UI::Theme
