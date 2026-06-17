#include "Toolbar.h"
#include <SDL3_image/SDL_image.h>
#include <string>
namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 ToolbarBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

Toolbar::Toolbar(int w, int h) : m_w(w), m_h(h) {}

Toolbar::~Toolbar() {
  for (int i = 0; i < TotalButtons; ++i) {
    if (m_textures[i] != nullptr) {
      SDL_DestroyTexture(m_textures[i]);
      m_textures[i] = nullptr;
    }
  }
}
struct ToolButton {
  ToolType type;
  const char *iconName;
};
void Toolbar::raisedBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::WHITE, 2.0f); // Top
  drawlist->AddLine(min, {min.x, max.y}, Theme::WHITE, 2.0f); // Left
  drawlist->AddLine({min.x, max.y}, max, Theme::BLACK, 2.0f); // Bottom
  drawlist->AddLine({max.x, min.y}, max, Theme::BLACK, 2.0f); // Right
}

void Toolbar::sunkenBorder(ImDrawList *drawlist, ImVec2 min, ImVec2 max) {
  drawlist->AddLine(min, {max.x, min.y}, Theme::BLACK, 2.0f); // Top
  drawlist->AddLine(min, {min.x, max.y}, Theme::BLACK, 2.0f); // Left
  drawlist->AddLine({min.x, max.y}, max, Theme::WHITE, 2.0f); // Bottom
  drawlist->AddLine({max.x, min.y}, max, Theme::WHITE, 2.0f); // Right
}
constexpr ToolButton kButtons[] = {
    {ToolType::FreeSelect, "01_free_form_select"},
    {ToolType::RectSelect, "02_select"},
    {ToolType::Eraser, "03_eraser"},
    {ToolType::FloodFill, "04_fill_bucket"},
    {ToolType::Eyedropper, "05_pick_color"},
    {ToolType::Magnifier, "06_magnifier"},
    {ToolType::Pencil, "07_pencil"},
    {ToolType::Brush, "08_brush"},
    {ToolType::Airbrush, "09_airbrush"},
    {ToolType::Text, "10_text"},
    {ToolType::Line, "11_line"},
    {ToolType::Curve, "12_curve"},
    {ToolType::Rectangle, "13_rectangle"},
    {ToolType::Polygon, "14_polygon"},
    {ToolType::Ellipse, "15_ellipse"},
    {ToolType::RoundedRectangle, "16_rounded_rectangle"},
};
bool Toolbar::init(SDL_Renderer *renderer) {
  const std::string tool_names[TotalButtons] = {"01_free_form_select",
                                                "02_select",
                                                "03_eraser",
                                                "04_fill_bucket",
                                                "05_pick_color",
                                                "06_magnifier",
                                                "07_pencil",
                                                "08_brush",
                                                "09_airbrush",
                                                "10_text",
                                                "11_line",
                                                "12_curve",
                                                "13_rectangle",
                                                "14_polygon",
                                                "15_ellipse",
                                                "16_rounded_rectangle"};

  bool all_loaded = true;
  for (int i = 0; i < TotalButtons; ++i) {
    std::string filepath =
        "../tools_icons/" + std::string(kButtons[i].iconName) + ".png";
    m_textures[i] = IMG_LoadTexture(renderer, filepath.c_str());

    if (m_textures[i] == nullptr) {
      SDL_Log("Toolbar: failed to load '%s' — %s", filepath.c_str(),
              SDL_GetError());
      all_loaded = false;
    }
  }
  return all_loaded;
}

void Toolbar::render() {
  constexpr ImVec2 ButtonPadding = {1.0f, 1.0f};
  const float buttonSide = 24.0f;

  const int columns = 2;
  const int rows = 8;
  const int totalButtons = columns * rows;

  const float toolbarWidth =
      (columns * buttonSide) + ((columns - 1) * 2.0f) + 15.0f;
  const float toolbarHeight =
      (rows * buttonSide) + ((rows - 1) * 2.0f) + 2.0f + 400.0f;
  // ImGui::GetStyle().FramePadding = ButtonPadding;
  ImGuiViewport *vp = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y + 22.0f});
  ImGui::SetNextWindowSize({toolMax.x, toolMax.y});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                      {2.0f, 1.0f}); // 4px left, handled via content offset
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {1.0f, 1.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ButtonPadding);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::ToolbarBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  ImGui::Begin("ToolbarGrid", nullptr,
               ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

  ImDrawList *drawlist = ImGui::GetWindowDrawList();
  // 4px left indent, 2px right effectively from window sizing
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);
  float startX = ImGui::GetCursorPosX();

  ImVec2 buttonSize = {buttonSide, buttonSide};

  for (int i = 0; i < totalButtons; ++i) {
    ImGui::PushID(i);

    // Re-apply left indent on every new row
    if ((i % columns) == 0 && i != 0) {
      ImGui::SetCursorPosX(startX);
    }

    ImTextureID tex = (ImTextureID)m_textures[i];

    ImGui::ImageButton("##icon", tex, buttonSize, ImVec2(0.0f, 0.0f),
                       ImVec2(1.0f, 1.0f), ImVec4(0, 0, 0, 0));

    ImVec2 btnMin = ImGui::GetItemRectMin();
    ImVec2 btnMax = ImGui::GetItemRectMax();

    bool isActive = (m_activeTool == kButtons[i].type);

    if (ImGui::IsItemClicked()) {
      m_activeTool = kButtons[i].type;
      isActive = true;
    }

    // Bevel: sunken if active or being pressed, raised otherwise
    if (ImGui::IsItemActive() || isActive) {
      sunkenBorder(drawlist, btnMin, btnMax);
    } else {
      raisedBorder(drawlist, btnMin, btnMax);
    }

    ImGui::PopID();

    if ((i + 1) % columns != 0) {
      ImGui::SameLine();
    }
  }

  raisedBorder(drawlist, toolMin, toolMax);
  ImGui::End();

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);

  // SDL_Log("%f,%f,%f,%f", toolMin.x, toolMin.y, toolMax.x, toolMax.y);
}

} // namespace UI
