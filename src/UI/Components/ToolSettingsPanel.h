#include <functional>

#include "UI/Components/UIComponents.h"
#include "UI/Layout/UILayout.h"
#include "UI/Styling/UIStyle.h"
#include "imgui.h"

class ToolSettingsPanel {
public:
  int brushSize = 1;
  int brushMin = 1;
  int brushMax = 32;

  bool eraserMode = false;

  std::function<void(int)> onSizeChanged;
  std::function<void(bool)> onEraserToggle;

  void render(ImVec2 position) {
    constexpr float panelW = 150.f;
    constexpr float rowH = 180.f;
    constexpr float padding = UIStyle::PanelPaddingX;

    const float panelH =
        rowH * 3.f + UIStyle::GroupSpacing * 4.f + UIStyle::PanelPaddingY * 2.f;
    UI::Panel::Desc desc;
    desc.pos = position;
    desc.size = {panelW, panelH};

    UI::Panel::show("##ToolSettingsPanel", desc, [&] {
      UI::VerticalPanel vp({0.f, 0.f}, {panelW, panelH});

      ImDrawList *draw = ImGui::GetWindowDrawList();
      ImVec2 winPos = ImGui::GetWindowPos();

      {
        ImVec2 pos = vp.reserve(panelH);
        UI::Label lbl{"brush Size"};
        ImVec2 labelPos = {winPos.x + pos.x,
                           winPos.y + pos.y + (panelH - lbl.size().y) * .5f};

        lbl.draw(draw, labelPos);

        ImGui::SameLine(80.f);

        ImGui::SetNextItemWidth(panelW - 80.f - padding * 2.f);
        int tmp = brushSize;
        if (ImGui::DragInt("##BrushSize", &tmp, 1, brushMin, brushMax)) {
          brushSize = tmp;
          if (onSizeChanged)
            onSizeChanged(brushSize);
        }
      }
      vp.addSpacing(UIStyle::ItemSpacing);

      vp.addSeparator(draw);
      // row 2
      {
        ImVec2 pos = vp.reserve(UIStyle::ToggleButtonHeight);
        UI::ToggleButton tgl{
            "eraser",
            &eraserMode,
            {panelW - padding * 2.f, UIStyle::ToggleButtonHeight}};
        ImGui::SetCursorPos({pos.x, pos.y});
        if (tgl.draw(draw) && onEraserToggle) {
          onEraserToggle(eraserMode);
        }
      }
    });
  }
};
