#include "Dropdown.h"
#include "UI/HoverStatus.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
//  Local colour palette (Win95 system colours)
// ─────────────────────────────────────────────────────────────────────────────

namespace {

constexpr ImU32 kBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 kHighlight = IM_COL32(0, 0, 128, 255);
constexpr ImU32 kHiText = IM_COL32(255, 255, 255, 255);
constexpr ImU32 kNormText = IM_COL32(0, 0, 0, 255);
constexpr ImU32 kDisabled = IM_COL32(128, 128, 128, 255);
constexpr ImU32 kDisabledLo = IM_COL32(255, 255, 255, 255);
constexpr ImU32 kSeparatorHi = IM_COL32(255, 255, 255, 255);
constexpr ImU32 kSeparatorLo = IM_COL32(128, 128, 128, 255);
constexpr ImU32 kBorderHi = IM_COL32(255, 255, 255, 255);
constexpr ImU32 kBorderLo = IM_COL32(64, 64, 64, 255);
constexpr ImU32 kBorderOuter = IM_COL32(0, 0, 0, 255);

} // namespace

using namespace UI::Layout;

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

Dropdown::Dropdown(std::string title, std::vector<MenuItem> items,
                   std::string hoverKey)
    : m_title(std::move(title)), m_items(std::move(items)),
      m_hoverKey(std::move(hoverKey)) {}

void Dropdown::setChecked(size_t index, bool checked) {
  if (index < m_items.size())
    m_items[index].checked = checked;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Layout helpers
// ─────────────────────────────────────────────────────────────────────────────

float Dropdown::computePanelWidth() const {
  float maxW = DropdownMinWidth;
  for (const auto &item : m_items) {
    if (item.type == MenuItemType::Separator)
      continue;
    float textW = ImGui::CalcTextSize(item.text.c_str()).x;
    float shortcutW = item.shortcut.empty()
                          ? 0.f
                          : ImGui::CalcTextSize(item.shortcut.c_str()).x;
    float rowW =
        DropdownIconColW + DropdownTextPadL + textW +
        (shortcutW > 0 ? DropdownShortcutPadR * 2.f + shortcutW : 0.f) +
        (item.type == MenuItemType::Submenu ? DropdownSubmenuArrowW : 0.f) +
        DropdownShortcutPadR;
    maxW = std::max(maxW, rowW);
  }
  return maxW;
}

float Dropdown::computePanelHeight() const {
  float h = DropdownPaddingY * 2.f;
  for (const auto &item : m_items)
    h += (item.type == MenuItemType::Separator) ? DropdownSeparatorH
                                                : DropdownItemHeight;
  return h;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Border helpers
// ─────────────────────────────────────────────────────────────────────────────

void Dropdown::drawRaisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const {
  dl->AddRect(min, max, kBorderOuter, 0.f, 0, 1.f);
  ImVec2 i0 = {min.x + 1, min.y + 1};
  ImVec2 i1 = {max.x - 1, max.y - 1};
  dl->AddLine(i0, {i1.x, i0.y}, kBorderHi, 1.f);
  dl->AddLine(i0, {i0.x, i1.y}, kBorderHi, 1.f);
  dl->AddLine({i0.x, i1.y}, i1, kBorderLo, 1.f);
  dl->AddLine({i1.x, i0.y}, i1, kBorderLo, 1.f);
}

void Dropdown::drawSunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) const {
  dl->AddRect(min, max, kBorderOuter, 0.f, 0, 1.f);
  ImVec2 i0 = {min.x + 1, min.y + 1};
  ImVec2 i1 = {max.x - 1, max.y - 1};
  dl->AddLine(i0, {i1.x, i0.y}, kBorderLo, 1.f);
  dl->AddLine(i0, {i0.x, i1.y}, kBorderLo, 1.f);
  dl->AddLine({i0.x, i1.y}, i1, kBorderHi, 1.f);
  dl->AddLine({i1.x, i0.y}, i1, kBorderHi, 1.f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Ribbon button
// ─────────────────────────────────────────────────────────────────────────────

bool Dropdown::renderRibbonButton(ImDrawList *dl, ImVec2 btnMin, ImVec2 btnMax,
                                  bool isActive) {
  const ImVec2 mousePos = ImGui::GetMousePos();
  const bool hovered = mousePos.x >= btnMin.x && mousePos.x < btnMax.x &&
                       mousePos.y >= btnMin.y && mousePos.y < btnMax.y;
  const bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

  if (isActive || (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)))
    drawSunkenBorder(dl, btnMin, btnMax);
  else if (hovered)
    drawRaisedBorder(dl, btnMin, btnMax);

  // ── Hover → push footer message ──────────────────────────────────────
  // Ribbon top-level buttons use the dropdown's own hoverKey when supplied,
  // otherwise we push the menu name so at least the generic message fires.
  if (hovered && !m_hoverKey.empty())
    HoverStatus::push(m_hoverKey);

  const ImVec2 textSize = ImGui::CalcTextSize(m_title.c_str());
  ImVec2 textPos = {
      btnMin.x + std::floor((btnMax.x - btnMin.x - textSize.x) * 0.5f),
      btnMin.y + std::floor((btnMax.y - btnMin.y - textSize.y) * 0.5f),
  };
  dl->AddText(textPos, kNormText, m_title.c_str());

  return clicked;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Item helpers
// ─────────────────────────────────────────────────────────────────────────────

void Dropdown::renderSeparator(ImDrawList *dl, ImVec2 rowMin,
                               float panelWidth) {
  float midY = rowMin.y + DropdownSeparatorH * 0.5f;
  float x0 = rowMin.x + DropdownIconColW;
  float x1 = rowMin.x + panelWidth - DropdownShortcutPadR;
  dl->AddLine({x0, midY}, {x1, midY}, kSeparatorLo, 1.f);
  dl->AddLine({x0, midY + 1}, {x1, midY + 1}, kSeparatorHi, 1.f);
}

void Dropdown::renderCheckmark(ImDrawList *dl, ImVec2 rowMin) const {
  float cx = rowMin.x + DropdownIconColW * 0.5f;
  float cy = rowMin.y + DropdownItemHeight * 0.5f;
  dl->AddLine({cx - 3, cy}, {cx, cy + 4}, kNormText, 2.f);
  dl->AddLine({cx, cy + 4}, {cx + 5, cy - 2}, kNormText, 2.f);
}

void Dropdown::renderSubmenuArrow(ImDrawList *dl, ImVec2 rowMin,
                                  float panelWidth) const {
  float right = rowMin.x + panelWidth - DropdownShortcutPadR;
  float cy = rowMin.y + DropdownItemHeight * 0.5f;
  dl->AddTriangleFilled({right - 6, cy - 4}, {right - 6, cy + 4}, {right, cy},
                        kNormText);
}

MenuAction Dropdown::renderItem(ImDrawList *dl, const MenuItem &item,
                                ImVec2 rowMin, float panelWidth, int index) {
  const ImVec2 rowMax = {rowMin.x + panelWidth, rowMin.y + DropdownItemHeight};
  const ImVec2 mousePos = ImGui::GetMousePos();

  const bool hovered = item.enabled && mousePos.x >= rowMin.x &&
                       mousePos.x < rowMax.x && mousePos.y >= rowMin.y &&
                       mousePos.y < rowMax.y;

  if (hovered) {
    m_hoveredIndex = index;

    // ── Hover → push footer message ──────────────────────────────────
    if (!item.hoverKey.empty())
      HoverStatus::push(item.hoverKey);
  }

  // Background
  if (hovered)
    dl->AddRectFilled(rowMin, rowMax, kHighlight);

  // Checkbox column
  if (item.type == MenuItemType::Checkbox && item.checked)
    renderCheckmark(dl, rowMin);

  // Label
  ImU32 textColor = hovered ? kHiText : (item.enabled ? kNormText : kDisabled);
  float textX = rowMin.x + DropdownIconColW + DropdownTextPadL;
  float textY =
      rowMin.y + (DropdownItemHeight - ImGui::GetTextLineHeight()) * 0.5f;

  if (!item.enabled)
    dl->AddText({textX + 1, textY + 1}, kDisabledLo, item.text.c_str());
  dl->AddText({textX, textY}, textColor, item.text.c_str());

  // Shortcut
  if (!item.shortcut.empty()) {
    ImVec2 scSize = ImGui::CalcTextSize(item.shortcut.c_str());
    float scX = rowMin.x + panelWidth - DropdownSubmenuArrowW -
                DropdownShortcutPadR - scSize.x;
    dl->AddText({scX, textY}, textColor, item.shortcut.c_str());
  }

  // Submenu arrow
  if (item.type == MenuItemType::Submenu)
    renderSubmenuArrow(dl, rowMin, panelWidth);

  // Click
  if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    if (item.type == MenuItemType::Submenu)
      m_openSubmenu = (m_openSubmenu == index) ? -1 : index;
    else
      return item.action;
  }

  // Nested submenu
  if (item.type == MenuItemType::Submenu && m_openSubmenu == index && hovered) {
    ImVec2 subOrigin = {rowMin.x + panelWidth - SubmenuOverlapX, rowMin.y};
    Dropdown sub("", item.children);
    sub.open();
    MenuAction subAction = sub.renderPanel(subOrigin);
    if (subAction != MenuAction::None) {
      m_openSubmenu = -1;
      return subAction;
    }
  }

  return MenuAction::None;
}

bool Dropdown::contains(ImVec2 origin, ImVec2 mouse) const {
  const float w = computePanelWidth();
  const float h = computePanelHeight();
  return mouse.x >= origin.x && mouse.x < origin.x + w && mouse.y >= origin.y &&
         mouse.y < origin.y + h;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Panel rendering
// ─────────────────────────────────────────────────────────────────────────────

MenuAction Dropdown::renderPanel(ImVec2 originPos) {
  if (!m_open)
    return MenuAction::None;

  const float panelW = computePanelWidth();
  const float panelH = computePanelHeight();

  const ImGuiViewport *vp = ImGui::GetMainViewport();
  if (originPos.x + panelW > vp->Pos.x + vp->Size.x)
    originPos.x = vp->Pos.x + vp->Size.x - panelW;
  if (originPos.y + panelH > vp->Pos.y + vp->Size.y)
    originPos.y -= panelH;

  const ImVec2 panelMax = {originPos.x + panelW, originPos.y + panelH};

  ImDrawList *dl = ImGui::GetForegroundDrawList();
  dl->AddRectFilled(originPos, panelMax, kBg);
  drawRaisedBorder(dl, originPos, panelMax);

  m_hoveredIndex = -1;

  MenuAction result = MenuAction::None;
  float cursorY = originPos.y + DropdownPaddingY;

  for (int i = 0; i < static_cast<int>(m_items.size()); ++i) {
    const MenuItem &item = m_items[i];
    ImVec2 rowMin = {originPos.x, cursorY};

    if (item.type == MenuItemType::Separator) {
      renderSeparator(dl, rowMin, panelW);
      cursorY += DropdownSeparatorH;
    } else {
      MenuAction act = renderItem(dl, item, rowMin, panelW, i);
      if (act != MenuAction::None)
        result = act;
      cursorY += DropdownItemHeight;
    }
  }

  return result;
}
