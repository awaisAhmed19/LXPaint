#pragma once
#include "UI/Styling/UIDrawHelper.h"
#include "UI/Styling/UIStyle.h"
#include <functional>
#include <imgui.h>

namespace UI {
struct PanelConext {
  ImDrawList *draw = nullptr;
  ImVec2 cursor = {};
  float height = 0.f;
  float width = 0.f;
};

class HorizontalPanel {
private:
  ImVec2 m_origin;
  ImVec2 m_size;
  ImVec2 m_cursor;
  float m_paddingX;
  float m_paddingY;

public:
  // origin = top-left of the panel in window - local coords
  explicit HorizontalPanel(ImVec2 origin, ImVec2 size,
                           float paddingX = UIStyle::PanelPaddingX,
                           float paddingY = UIStyle::PanelPaddingY)
      : m_origin(origin), m_size(size), m_paddingX(paddingX),
        m_paddingY(paddingY) {
    m_cursor = {origin.x, origin.y};
  }

  void addSpacing(float amount = UIStyle::GroupSpacing) {
    m_cursor.x += amount;
  }

  ImVec2 reserve(float itemWidth) {
    ImVec2 pos = m_cursor;
    ImGui::SetCursorPos({pos.x, pos.y + m_paddingY});
    m_cursor.x += itemWidth + UIStyle::ItemSpacing;
    return pos;
  }
  void addSeparator(ImDrawList *draw) {
    float x = m_cursor.x + UIStyle::SeparatorMarginX;
    float yT = m_origin.y;
    float yB = m_origin.y + height();
    UIDrawHelpers::drawVerticalSeparator(draw, x, yT, yB);
    m_cursor.x += UIStyle::SeparatorMarginX * 2.f + 2.f + UIStyle::GroupSpacing;
  }

  float height() const {
    return m_size.y > 0.f ? m_size.y
                          : ImGui::GetTextLineHeight() + m_paddingY * 2.5f;
  }

  float currentX() const { return m_cursor.x; }
  ImVec2 origin() const { return m_origin; }
};

class VerticalPanel {
private:
  ImVec2 m_origin;
  ImVec2 m_size;
  ImVec2 m_cursor;
  float m_paddingX;
  float m_paddingY;

public:
  explicit VerticalPanel(ImVec2 origin, ImVec2 size,
                         float paddingX = UIStyle::PanelPaddingX,
                         float paddingY = UIStyle::PanelPaddingY)
      : m_origin(origin), m_size(size), m_paddingX(paddingX),
        m_paddingY(paddingY) {
    m_cursor = {origin.x + paddingX, origin.y + paddingY};
  }

  ImVec2 reserve(float itemHeight) {
    ImVec2 pos = m_cursor;
    ImGui::SetCursorPos({pos.x + m_paddingX, pos.y});
    m_cursor.y += itemHeight + UIStyle::ItemSpacing;
    return pos;
  }
  void addSpacing(float amount = UIStyle::GroupSpacing) {
    m_cursor.x += amount;
  }
  void addSeparator(ImDrawList *draw) {
    float y = m_cursor.y + UIStyle::SeparatorMarginY;
    float xL = m_origin.x;
    float xR = m_origin.x + width();
    draw->AddLine({xL, y}, {xR, y}, UIStyle::colorBorder());
    draw->AddLine({xL, y + 1}, {xR, y + 1}, UIStyle::colorHighlight());

    m_cursor.y += UIStyle::SeparatorMarginY * 2.f + 2.f + UIStyle::GroupSpacing;
  }

  float width() const { return m_size.x > 0.f ? m_size.x : 120.f; }

  float currentY() const { return m_cursor.y; }
  ImVec2 origin() const { return m_origin; }
};
class ToolbarGroup {
private:
  const char *m_name;
  HorizontalPanel &m_panel;
  ImDrawList *m_draw;

public:
  ToolbarGroup(const char *name, HorizontalPanel &panel, ImDrawList *draw)
      : m_name(name), m_panel(panel), m_draw(draw) {
    m_panel.addSpacing(UIStyle::GroupSpacing * .5f);
  }

  void item(float reverseWidth, std::function<void(ImDrawList *)> itemfnc) {
    m_panel.reserve(reverseWidth);
    itemfnc(m_draw);
  }

  void end() { m_panel.addSeparator(m_draw); }
  const char *name() const { return m_name; }
};

class Panel {
public:
  struct Desc {
    ImVec2 pos;
    ImVec2 size;
    bool noTitleBar = true;
    bool noResize = true;
    bool noMove = true;
    bool noScrollbar = false;
    bool noBring = true;
  };

  static void show(const char *id, const Desc &desc,
                   std::function<void()> content) {
    ImGui::SetNextWindowPos(desc.pos, ImGuiCond_Always);
    if (desc.size.x > 0.f || desc.size.y > 0.f)
      ImGui::SetNextWindowSize(desc.size, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings;
    if (desc.noTitleBar)
      flags |= ImGuiWindowFlags_NoTitleBar;
    if (desc.noResize)
      flags |= ImGuiWindowFlags_NoResize;
    if (desc.noMove)
      flags |= ImGuiWindowFlags_NoMove;
    if (desc.noScrollbar)
      flags |= ImGuiWindowFlags_NoScrollbar;
    if (desc.noBring)
      flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin(id, nullptr, flags)) {
      content();
    }
    ImGui::End();
  }
};
}; // namespace UI
