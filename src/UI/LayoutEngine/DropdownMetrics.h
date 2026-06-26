#pragma once

namespace UI::Layout {

// ─────────────────────────────────────────────────────────────────────────────
//  DropdownMetrics
//
//  All spatial constants for the Windows-95-style dropdown menus.
//  Keep every magic number here so Dropdown.cpp and Ribbon.cpp
//  never contain raw literals.
// ─────────────────────────────────────────────────────────────────────────────

// Popup panel
inline constexpr float DropdownMinWidth      = 200.0f;
inline constexpr float DropdownItemHeight    = 19.0f;   // normal row
inline constexpr float DropdownSeparatorH    =  8.0f;   // thin rule row
inline constexpr float DropdownPaddingY      =  2.0f;   // top/bottom inset
inline constexpr float DropdownBorderWidth   =  1.0f;

// Left column widths inside an item row
inline constexpr float DropdownIconColW      = 20.0f;   // checkbox / icon area
inline constexpr float DropdownTextPadL      =  4.0f;   // gap after icon col
inline constexpr float DropdownShortcutPadR  = 12.0f;   // gap before right edge
inline constexpr float DropdownSubmenuArrowW = 14.0f;   // ">" indicator

// Submenu horizontal offset (overlaps parent by this many px, classic Win95)
inline constexpr float SubmenuOverlapX       =  2.0f;

// Shadow-like outer border — Win95 used a 2-px raised/sunken combo
inline constexpr float DropdownShadowOffset  =  2.0f;

// Colours (ARGB uint32 helpers are in Dropdown.cpp; these are the semantic names)
// Kept as comments so Dropdown.cpp can define them as constexpr ImU32 locally.
//
//   DropdownBg         = IM_COL32(192, 192, 192, 255)  -- classic gray
//   DropdownHighlight  = IM_COL32(  0,   0, 128, 255)  -- navy selection bar
//   DropdownHiText     = IM_COL32(255, 255, 255, 255)
//   DropdownNormText   = IM_COL32(  0,   0,   0, 255)
//   DropdownDisabled   = IM_COL32(128, 128, 128, 255)
//   DropdownSeparator  = IM_COL32(128, 128, 128, 255)
//   DropdownBorderHi   = IM_COL32(255, 255, 255, 255)  -- raised-border light
//   DropdownBorderLo   = IM_COL32( 64,  64,  64, 255)  -- raised-border dark

} // namespace UI::Layout
