#include "TopRibbon.h"
#include "UI/Components/UIComponents.h"
#include "UI/Layout/UILayout.h"
#include "UI/Styling/UIDrawHelper.h"
#include "UI/Styling/UIStyle.h"
#include "imgui.h"

TopRibbon::TopRibbon() {
  m_menus = {
      {"File", MenuType::File, [] {}},   {"Edit", MenuType::Edit, [] {}},
      {"View", MenuType::View, [] {}},   {"Image", MenuType::Image, [] {}},
      {"Color", MenuType::Color, [] {}}, {"Help", MenuType::Help, [] {}},
  };
}

void TopRibbon::bindMenu(MenuType type, std::function<void()> fn) {
  for (auto &entry : m_menus) {
    if (entry.type == type) {
      entry.onActivate = std::move(fn);
    }
  }
}

void TopRibbon::drawBackground(ImDrawList *draw, ImVec2 rbMin,
                               ImVec2 rbMax) const {
  draw->AddRectFilled(rbMin, rbMax, UIStyle::colorSurface());
  UIDrawHelpers::drawRaisedBorder(draw, rbMin, rbMax);
}

void TopRibbon::render() {
  ImGuiViewport *vp = ImGui::GetMainViewport();

  const float fontH = ImGui::GetTextLineHeight();
  const float ribbonH = fontH + UIStyle::RibbonPaddingY * 2.5f;
  const float ribbonW = vp->Size.x;

  ImVec2 rbMin = vp->Pos;
  ImVec2 rbMax = {vp->Pos.x + ribbonW, vp->Pos.y + ribbonH};

  ImDrawList *draw = ImGui::GetForegroundDrawList();
  drawBackground(draw, rbMin, rbMax);

  ImGui::SetNextWindowPos(rbMin);
  ImGui::SetNextWindowSize({ribbonW, ribbonH});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

  ImGui::Begin("##TopRibbonHitArea", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBackground |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  UI::HorizontalPanel panel({0.f, 0.f}, {ribbonW, ribbonH});

  // ── File / menu group ─────────────────────────────────────
  {
    UI::ToolbarGroup group("Menus", panel, draw);

    for (auto &entry : m_menus) {
      const float btnW = UIStyle::RibbonButtonWidth;
      UI::Button btn{entry.label, {btnW, ribbonH}};

      group.item(btnW, [&](ImDrawList *dl) {
        if (btn.draw(dl) && entry.onActivate) {
          entry.onActivate();
        }
      });
    }

    // group.end(); // trailing separator
  }

  ImGui::End();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(2);
}
