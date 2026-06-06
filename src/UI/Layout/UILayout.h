#pragma once
#include "UI/Styling/UIDrawHelper.h"
#include "UI/Styling/UIStyle.h"
#include <functional>
#include <imgui.h>

namespace UI {

struct LayoutBox {
  ImVec2 min;
  ImVec2 max;

  float marginX = 0.f;
  float marginY = 0.f;
  float paddingX = 0.f;
  float paddingY = 0.f;

  LayoutBox(ImVec2 origin, ImVec2 size, float px = 0.f, float py = 0.f,
            float mx = 0.f, float my = 0.f)
      : min(origin), max({origin.x + size.x, origin.y + size.y}), marginX(mx),
        marginY(my), paddingX(px), paddingY(py) {}

  ImVec2 size() const { return {max.x - min.x, max.y - min.y}; }
  ImVec2 pos() const { return min; }

  // Content area (inset by padding)
  ImVec2 contentMin() const { return {min.x + paddingX, min.y + paddingY}; }
  ImVec2 contentMax() const { return {max.x - paddingX, max.y - paddingY}; }
  ImVec2 contentSize() const {
    return {contentMax().x - contentMin().x, contentMax().y - contentMin().y};
  }

  // Position for next element (accounting for margin)
  ImVec2 nextPos(bool vertical = false) const {
    if (vertical)
      return {min.x, max.y + marginY};
    return {max.x + marginX, min.y};
  }

  // Create a box without margins applied
  LayoutBox withoutMargins() const {
    ImVec2 adjustedSize = {size().x - marginX * 2.f, size().y - marginY * 2.f};
    return LayoutBox({min.x + marginX, min.y + marginY}, adjustedSize, paddingX,
                     paddingY);
  }
};

class LayoutCursor {
private:
  ImVec2 m_cursor;
  ImVec2 m_containerStart;
  float m_containerWidth;
  bool m_vertical;

public:
  LayoutCursor(ImVec2 start, float containerWidth, bool vertical = false)
      : m_cursor(start), m_containerStart(start),
        m_containerWidth(containerWidth), m_vertical(vertical) {}

  /**
   * Reserve space for an item and advance cursor
   * Returns: LayoutBox with position and size information
   */
  LayoutBox reserve(ImVec2 size, float paddingX = UIStyle::PanelPaddingX,
                    float paddingY = UIStyle::PanelPaddingY,
                    float marginX = UIStyle::ItemSpacing,
                    float marginY = UIStyle::ItemSpacing) {
    LayoutBox box(m_cursor, size, paddingX, paddingY, marginX, marginY);

    // Advance cursor for next item
    if (m_vertical) {
      m_cursor.y += size.y + marginY;
    } else {
      m_cursor.x += size.x + marginX;
    }

    return box;
  }

  void newline(float lineHeight, float marginY = UIStyle::ItemSpacing) {
    if (m_vertical)
      return;

    m_cursor.x = m_containerStart.x;
    m_cursor.y += lineHeight + marginY;
  }

  void addSpacing(float amount) {
    if (m_vertical) {
      m_cursor.y += amount;
    } else {
      m_cursor.x += amount;
    }
  }

  ImVec2 position() const { return m_cursor; }
  ImVec2 containerStart() const { return m_containerStart; }
};

class PanelBuilder {
public:
  struct config {
    ImVec2 pos;
    ImVec2 size;
    float pX = UIStyle::PanelPaddingX;
    float pY = UIStyle::PanelPaddingY;
    ImU32 bgColor = UIStyle::colorSurface();
    bool drawBorder = true;
    bool vertical = false;
    const char *id = "##panel";
  };

  struct ItemConfig {
    float paddingX = UIStyle::PanelPaddingX;
    float paddingY = UIStyle::PanelPaddingY;
    float marginX = UIStyle::ItemSpacing;
    float marginY = UIStyle::ItemSpacing;
  };

private:
  config m_config;
  ImDrawList *m_draw;
  LayoutCursor m_cursor;
  ImVec2 m_winPos;

public:
  PanelBuilder(const config &cfg)
      : m_config(cfg), m_draw(nullptr),
        m_cursor({cfg.pos.x + cfg.pX, cfg.pos.y + cfg.pY}, cfg.size.x,
                 cfg.vertical) {
    m_draw = ImGui::GetWindowDrawList();
    m_winPos = ImGui::GetWindowPos();
  }

  PanelBuilder &drawBackground() {
    LayoutBox box(m_config.pos, m_config.size);
    m_draw->AddRectFilled(box.min, box.max, m_config.bgColor);
    if (m_config.drawBorder)
      UIDrawHelpers::drawRaisedBorder(m_draw, box.min, box.max);
    return *this;
  }

  PanelBuilder &Button(const char *label, const ItemConfig &cfg,
                       std::function<void()> onClick) {
    LayoutBox box = m_cursor.reserve({50.f, 20.f}, cfg.paddingX, cfg.paddingY,
                                     cfg.marginX, cfg.marginY);

    ImGui::SetCursorPos({box.min.x - m_winPos.x, box.min.y - m_winPos.y});

    if (ImGui::InvisibleButton(label, box.size())) {
      if (onClick) {
        onClick();
      }
    }

    bool onHover = ImGui::IsItemHovered();
    bool onActive = ImGui::IsItemActive();

    m_draw->AddRectFilled(box.min, box.max, UIStyle::colorSurface());

    if (onActive) {
      UIDrawHelpers::drawSunkenBorder(m_draw, box.min, box.max);
    } else if (onHover) {
      UIDrawHelpers::drawRaisedBorder(m_draw, box.min, box.max);
    }

    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos = {box.min.x + (box.size().x - textSize.x) * 0.5f,
                      box.min.y + (box.size().y - textSize.y) * 0.5f};
    m_draw->AddText(textPos, UIStyle::colorText(), label);

    return *this;
  }

  PanelBuilder &label(const char *text, const ItemConfig &cfg,
                      ImU32 color = 0) {
    ImU32 col = (color != 0) ? color : UIStyle::colorText();
    ImVec2 textSize = ImGui::CalcTextSize(text);
    LayoutBox box = m_cursor.reserve(textSize, cfg.paddingX, cfg.paddingY,
                                     cfg.marginX, cfg.marginY);

    m_draw->AddText(box.contentMin(), col, text);
    return *this;
  }

  PanelBuilder &separator(float height = 20.f) {
    LayoutBox box = m_cursor.reserve({2.f, height});
    float x = box.min.x + 1.f;
    UIDrawHelpers::drawVerticalSeparator(m_draw, x, box.min.y, box.max.y);
    return *this;
  }

  PanelBuilder &spacing(float amount) {
    m_cursor.addSpacing(amount);
    return *this;
  }

  PanelBuilder &group(const char *name,
                      std::function<PanelBuilder &(PanelBuilder &)> content) {
    label(name, {});
    spacing(UIStyle::GroupSpacing * 0.5f);
    content(*this);
    separator();
    return *this;
  }

  PanelBuilder &custom(ImVec2 size, const ItemConfig &cfg,
                       std::function<void(LayoutBox)> drawer) {
    LayoutBox box = m_cursor.reserve(size, cfg.paddingX, cfg.paddingY,
                                     cfg.marginX, cfg.marginY);
    drawer(box);
    return *this;
  }

  ImVec2 position() const { return m_cursor.position(); }

  LayoutBox currentBox() const {
    return LayoutBox(m_config.pos, m_config.size, m_config.pX, m_config.pY);
  }
};

class LayoutFactory {
public:
  static PanelBuilder toolbar(ImVec2 position, ImVec2 size) {
    PanelBuilder::config cfg;
    cfg.pos = position;
    cfg.size = size;
    cfg.vertical = false;
    return PanelBuilder(cfg);
  }

  static PanelBuilder verticalPanel(ImVec2 position, ImVec2 size) {
    PanelBuilder::config cfg;
    cfg.pos = position;
    cfg.size = size;
    cfg.vertical = true;
    return PanelBuilder(cfg);
  }

  static PanelBuilder overlay(ImVec2 position, ImVec2 size) {
    PanelBuilder::config cfg;
    cfg.pos = position;
    cfg.size = size;
    cfg.drawBorder = false;
    cfg.bgColor = IM_COL32(0, 0, 0, 0);
    return PanelBuilder(cfg);
  }
};

} // namespace UI
