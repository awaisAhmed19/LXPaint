#include "Toolbar.h"
#include "App/Globals.h"
#include "Editor/Editor.h"
#include "UI/LayoutEngine/UILayoutConstant.h"
#include "imgui.h"
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
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
constexpr ImU32 OptionHovered = IM_COL32(0, 0, 50, 255);
constexpr ImU32 Selected = IM_COL32(0, 0, 128, 255);
} // namespace Theme

struct ToolOption {
  int buttonNumber = 0;
  ImVec2 pos = {0, 0};
  ImVec2 size = {0, 0};
};

struct ToolButton {
  ToolType type;
  const char *iconName;
  ToolOption tooloption;
};

static constexpr ToolButton kButtons[] = {
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

Toolbar::Toolbar(int w, int h, SDL_Renderer *renderer)
    : m_w(w), m_h(h), m_renderer(renderer) {}

Toolbar::~Toolbar() {
  for (int i = 0; i < TotalButtons; ++i) {
    if (m_textures[i]) {
      SDL_DestroyTexture(m_textures[i]);
      m_textures[i] = nullptr;
    }
  }
  if (m_backgroundTransparentIcon) {

    SDL_DestroyTexture(m_backgroundTransparentIcon);
    m_backgroundTransparentIcon = nullptr;
  }
  if (m_backgroundOpaqueIcon) {

    SDL_DestroyTexture(m_backgroundOpaqueIcon);
    m_backgroundOpaqueIcon = nullptr;
  }
  // doubtful if destroying renderer is wise perhaps the class m_renderer must
  // be nulled;
  if (m_renderer) {
    SDL_DestroyRenderer(m_renderer);
    m_renderer = nullptr;
  }
}

void Toolbar::raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max,
                           float thickness) {
  dl->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f);
  dl->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f);
  dl->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f);
}

void Toolbar::sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max,
                           float thickness) {
  dl->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  dl->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  dl->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}

bool Toolbar::init() {
  bool ok = true;
  for (int i = 0; i < TotalButtons; ++i) {
    std::string path =
        "../assets/tools_icons/" + std::string(kButtons[i].iconName) + ".png";

    m_textures[i] = IMG_LoadTexture(m_renderer, path.c_str());

    if (!m_textures[i]) {
      SDL_Log("Toolbar: failed to load '%s': %s", path.c_str(), SDL_GetError());
      ok = false;
    }
  }

  m_backgroundOpaqueIcon = IMG_LoadTexture(
      m_renderer, "../assets/tools_icons/options-transparency-top.png");

  m_backgroundTransparentIcon = IMG_LoadTexture(
      m_renderer, "../assets/tools_icons/options-transparency-bottom.png");
  if (!m_backgroundOpaqueIcon) {
    ok = false;
    SDL_Log("%s", SDL_GetError());
  }

  if (!m_backgroundTransparentIcon) {
    ok = false;
    SDL_Log("%s", SDL_GetError());
  }
  return ok;
}

//--------------------------------------------------------------------------
// Eraser — 1x4 grid of square swatches (eraser is square, unlike brush dots)
//--------------------------------------------------------------------------
void Toolbar::renderSizeSquares(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                float optionWidth, float optionHeight,
                                const int *sizes, int count) {
  constexpr float gap = 1.0f;

  float &target = editor.getToolSettings().eraserSize;

  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 3.0f) / 4.0f;

  for (int i = 0; i < count && i < 4; ++i) {
    ImVec2 btnMin = {origin.x + 1.f, (origin.y + 1) + i * (cellH + gap)};

    ImVec2 btnMax = {btnMin.x + cellW - 1.f, btnMin.y + cellH - 1.f};

    ImVec2 center = {(btnMin.x + btnMax.x) * 0.5f,
                     (btnMin.y + btnMax.y) * 0.5f};

    bool selected = (target == static_cast<float>(sizes[i]));

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    float half = std::min(cellH, cellW) * 0.04f * sizes[i];
    half = std::clamp(half, 2.0f, 10.0f);

    dl->AddRectFilled({center.x - half, center.y - half},
                      {center.x + half, center.y + half},
                      selected ? Theme::WHITE : Theme::BLACK);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 900);

    if (ImGui::InvisibleButton("##erasersize", {cellW, cellH}))
      target = static_cast<float>(sizes[i]);

    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Brush — 4x3 grid of shape stamps (round / square / fwd-slash / backslash)
//--------------------------------------------------------------------------
void Toolbar::renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                float optionWidth, float optionHeight) {
  constexpr float gap = 2.0f;
  constexpr float padding = 2.f;
  auto &brushShape = editor.getToolSettings().brushShape;
  auto &strokeWidth = editor.getToolSettings().strokeWidth;

  const float cellW = ((optionWidth - padding) - gap * 2.0f) / 3.0f;
  const float cellH = ((optionHeight - padding) - gap * 3.0f) / 4.0f;

  for (int i = 0; i < 12; ++i) {
    const int row = i / 3; // Brush shape
    const int col = i % 3; // Brush size

    ImVec2 btnMin = {origin.x + padding + col * (cellW),
                     origin.y + padding + row * (cellH)};

    ImVec2 btnMax = {btnMin.x + cellW, btnMin.y + cellH};

    float radius;
    float thickness;
    float widthValue;

    switch (col) {
    case 0:
      radius = 2.0f;
      thickness = 1.f;
      widthValue = 1.0f;
      break;

    case 1:
      radius = 4.0f;
      thickness = 2.f;
      widthValue = 2.0f;
      break;

    default:
      radius = 6.0f;
      thickness = 3.f;
      widthValue = 3.0f;
      break;
    }

    bool selected = brushShape == static_cast<ToolSettings::BrushShape>(row) &&
                    strokeWidth == widthValue;

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    ImVec2 center = {(btnMin.x + btnMax.x) * 0.5f,
                     (btnMin.y + btnMax.y) * 0.5f};

    switch (row) {
    case 0: // Round
      dl->AddCircleFilled(center, radius,
                          selected ? Theme::WHITE : Theme::BLACK);
      break;

    case 1: // Square
      dl->AddRectFilled({center.x - radius, center.y - radius},
                        {center.x + radius, center.y + radius},
                        selected ? Theme::WHITE : Theme::BLACK);
      break;

    case 2: // Forward Slash
      dl->AddLine({center.x - radius, center.y + radius},
                  {center.x + radius, center.y - radius},
                  selected ? Theme::WHITE : Theme::BLACK, thickness);
      break;

    case 3: // Back Slash
      dl->AddLine({center.x - radius, center.y - radius},
                  {center.x + radius, center.y + radius},
                  selected ? Theme::WHITE : Theme::BLACK, thickness);
      break;
    }

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 300);

    if (ImGui::InvisibleButton("##brush", {cellW, cellH})) {
      brushShape = static_cast<ToolSettings::BrushShape>(row);
      strokeWidth = widthValue;
    }

    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Rectangle / Ellipse / Polygon / RoundedRectangle — outline/fill/both
//--------------------------------------------------------------------------
void Toolbar::renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                              float optionWidth, float optionHeight) {
  constexpr float gap = 1.0f;

  auto &mode =
      editor.getToolSettings().brushShape; // TODO: replace with fillMode

  struct FillPreview {
    const char *id;
    bool outline;
    bool filled;
  };

  static constexpr FillPreview kModes[3] = {
      {"##fm0", true, false}, // Outline
      {"##fm1", false, true}, // Filled
      {"##fm2", true, true},  // Outline + Filled
  };

  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 2.0f) / 3.0f;

  for (int i = 0; i < 3; ++i) {
    ImVec2 btnMin = {origin.x + 1.0f, origin.y + 1.0f + i * (cellH + gap)};

    ImVec2 btnMax = {btnMin.x + cellW - 1.0f, btnMin.y + cellH - 1.0f};

    bool selected = (mode == static_cast<ToolSettings::BrushShape>(i));

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    constexpr float padX = 8.0f;
    constexpr float padY = 4.0f;

    ImVec2 rectMin = {btnMin.x + padX, btnMin.y + padY};
    ImVec2 rectMax = {btnMax.x - padX, btnMax.y - padY};

    if (kModes[i].filled)
      dl->AddRectFilled(rectMin, rectMax, IM_COL32(128, 128, 128, 255));

    if (kModes[i].outline)
      dl->AddRect(rectMin, rectMax, IM_COL32(128, 128, 128, 255));

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 400);

    if (ImGui::InvisibleButton(kModes[i].id, {cellW, cellH}))
      mode = static_cast<ToolSettings::BrushShape>(i);

    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Line / Curve — 4 stacked thickness bars
//--------------------------------------------------------------------------
void Toolbar::renderLineWidths(Editor &editor, ImDrawList *dl, ImVec2 origin,
                               float optionWidth, float optionHeight) {
  static constexpr float kWidths[4] = {1.0f, 2.0f, 3.0f, 5.0f};
  constexpr float gap = 1.0f;

  float &target = editor.getToolSettings().lineWidth;

  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 3.0f) / 4.0f;

  for (int i = 0; i < 4; ++i) {
    ImVec2 btnMin = {origin.x + 1.0f, origin.y + 1.0f + i * (cellH + gap)};

    ImVec2 btnMax = {btnMin.x + cellW - 1.0f, btnMin.y + cellH - 1.0f};

    ImVec2 center = {(btnMin.x + btnMax.x) * 0.5f,
                     (btnMin.y + btnMax.y) * 0.5f};

    bool selected = (target == kWidths[i]);

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    dl->AddLine({btnMin.x + 6.0f, center.y}, {btnMax.x - 6.0f, center.y},
                selected ? Theme::WHITE : Theme::BLACK, kWidths[i]);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 500);

    if (ImGui::InvisibleButton("##linewidth", {cellW, cellH}))
      target = kWidths[i];

    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Airbrush — 3 stacked spray-density icons (small / medium / large)
//--------------------------------------------------------------------------
void Toolbar::renderAirbrushSizes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                  float optionWidth, float optionHeight) {
  static constexpr float kSizes[3] = {5.0f, 8.0f, 12.0f};
  static constexpr int kDotCounts[3] = {10, 18, 28};
  const float gap = 2.0f;
  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 3.0f) / 3.0f;

  float &target = editor.getToolSettings().airbrushRadius;

  for (int i = 0; i < 3; ++i) {
    ImVec2 btnMin = {origin.x + 1.0f, origin.y + 1.0f + i * (cellH + gap)};

    ImVec2 btnMax = {btnMin.x + cellW - 1.0f, btnMin.y + cellH - 1.0f};

    bool selected = (target == kSizes[i]);

    ImVec2 c = {(btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f};
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    float radius = kSizes[i] * 0.6f;

    // Deterministic pseudo-random spray dots (no <random> dependency)
    unsigned seed = 1234u + (unsigned)i * 97u;
    auto rnd = [&seed]() {
      seed = seed * 1103515245u + 12345u;
      return (float)((seed >> 16) & 0x7FFF) / 32767.0f;
    };
    for (int d = 0; d < kDotCounts[i]; ++d) {
      float ang = rnd() * 6.2831853f;
      float r = rnd() * radius;
      ImVec2 p = {c.x + std::cos(ang) * r, c.y + std::sin(ang) * r};
      dl->AddCircleFilled(p, 1.0f, selected ? Theme::WHITE : Theme::BLACK);
    }

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 600);
    if (ImGui::InvisibleButton("##spray", {cellW, cellH}))
      target = kSizes[i];
    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Magnifier — vertical list of zoom presets: 1x, 2x, 6x, 8x
//--------------------------------------------------------------------------
void Toolbar::renderZoomLevels(Editor &editor, ImDrawList *dl, ImVec2 origin,
                               float optionWidth, float optionHeight) {
  static constexpr int kLevels[4] = {1, 2, 6, 8};

  constexpr float gap = 2.0f;
  const float cellW = optionWidth;
  //  const float cellH = (optionHeight - gap * 3.0f) / 4.0f;
  const float cellH = (optionHeight) / 4.0f;

  int &target = editor.getToolSettings().zoomLevel;

  for (int i = 0; i < 4; ++i) {
    ImVec2 btnMin = {origin.x + 1.0f, origin.y + 1.0f + i * (cellH + gap)};

    ImVec2 btnMax = {btnMin.x + cellW - 1.0f, btnMin.y + cellH - 1.0f};

    bool selected = (target == kLevels[i]);

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    char label[8];
    std::snprintf(label, sizeof(label), "%dx", kLevels[i]);

    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos = {(btnMin.x + btnMax.x - textSize.x) * 0.5f,
                      (btnMin.y + btnMax.y - textSize.y) * 0.5f};

    dl->AddText(textPos, selected ? Theme::WHITE : Theme::BLACK, label);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 800);

    if (ImGui::InvisibleButton("##zoom", {cellW, cellH}))
      target = kLevels[i];

    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
// Selection (FreeSelect/RectSelect) + Text — opaque/transparent background
//--------------------------------------------------------------------------
void Toolbar::renderBackgroundModeIcons(Editor &editor, ImDrawList *dl,
                                        ImVec2 origin, float optionWidth,
                                        float optionHeight,
                                        ToolSettings::BackgroundMode &target) {
  constexpr float gap = 0.0f;

  const float cellW = optionWidth;
  const float cellH = optionHeight / 2.0f;

  ImTextureID icons[2] = {(ImTextureID)m_backgroundOpaqueIcon,
                          (ImTextureID)m_backgroundTransparentIcon};

  for (int i = 0; i < 2; ++i) {
    ToolSettings::BackgroundMode mode =
        (i == 0) ? ToolSettings::BackgroundMode::Opaque
                 : ToolSettings::BackgroundMode::Transparent;

    ImVec2 btnMin = {origin.x + 1.0f, origin.y + 1.0f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 3.0f, btnMin.y + cellH - 3.0f};

    bool selected = (target == mode);

    // Draw background
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);

    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);
    constexpr float pad = 5.f;

    float iconW = cellW - 2.0f * pad;
    float iconH = cellH - 2.0f * pad;

    ImGui::SetCursorScreenPos({btnMin.x, btnMin.y});

    ImGui::PushID(i + 700);

    ImGui::ImageButton("##bgmode", icons[i], {iconW, iconH}, {0.f, 0.f},
                       {1.f, 1.f}, ImVec4(0, 0, 0, 0));

    if (ImGui::IsItemClicked())
      target = mode;
    ImGui::PopID();
  }
}

//--------------------------------------------------------------------------
void Toolbar::renderOptions(Editor &editor, ImDrawList *dl) {
  ImVec2 winPos = ImGui::GetWindowPos();
  float boxX = winPos.x + 4.0f;
  float boxY = winPos.y + 222.0f;
  float boxW = 58.0f;
  float boxH = 80.0f;

  ImVec2 boxMin = {boxX, boxY};
  ImVec2 boxMax = {boxX + boxW, boxY + boxH};
  dl->AddRectFilled(boxMin, boxMax, IM_COL32(192, 192, 192, 255));
  sunkenBorder(dl, boxMin, boxMax);

  ImVec2 inner = {boxX, boxY};

  static constexpr int kEraserSizes[] = {4, 6, 8, 10};

  switch (editor.getActiveTool()) {

  case ToolType::Eraser:
    renderSizeSquares(editor, dl, inner, boxW, boxH, kEraserSizes, 4);
    break;

  case ToolType::Line:
  case ToolType::Curve:
    renderLineWidths(editor, dl, inner, boxW, boxH);
    break;

  case ToolType::Airbrush:
    renderAirbrushSizes(editor, dl, inner, boxW, boxH);
    break;

  case ToolType::Brush:
    renderBrushShapes(editor, dl, inner, boxW, boxH);
    break;

  case ToolType::Rectangle:
  case ToolType::Ellipse:
  case ToolType::RoundedRectangle:
  case ToolType::Polygon:
    renderFillModes(editor, dl, inner, boxW, boxH);
    break;

  case ToolType::FreeSelect:
  case ToolType::RectSelect:
  case ToolType::Text:
    renderBackgroundModeIcons(editor, dl, inner, boxW, boxH,
                              editor.getToolSettings().backgroundMode);
    break;

  case ToolType::Magnifier:
    renderZoomLevels(editor, dl, inner, boxW, boxW);
    break;

  default:
    // Pencil, FloodFill, Eyedropper: no options
    break;
  }
}

float Toolbar::preferredWidth() const { return UI::Layout::ToolbarWidth; }

void Toolbar::render(Editor &editor) {
  constexpr int kColumns = 2;
  constexpr int kRows = 8;
  constexpr float kButtonSize = 24.0f;
  constexpr float kButtonGap = 1.0f;
  constexpr float kRibbonHeight = 22.0f;
  constexpr float kLeftInset = 4.0f;

  constexpr ImVec2 kWindowPadding{2.0f, 0.0f};
  constexpr ImVec2 kItemSpacing{kButtonGap, kButtonGap};
  constexpr ImVec2 kFramePadding{1.0f, 1.0f};

  constexpr ImGuiWindowFlags kFlags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGuiViewport *vp = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y + kRibbonHeight},
                          ImGuiCond_Always);
  ImGui::SetNextWindowSize({toolMax.x, vp->Size.y * 0.849f}, ImGuiCond_Always);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, kWindowPadding);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kItemSpacing);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, kFramePadding);

  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::ToolbarBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  ImGui::Begin("ToolbarGrid", nullptr, kFlags);

  ImDrawList *dl = ImGui::GetWindowDrawList();

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + kLeftInset);
  const float startX = ImGui::GetCursorPosX();

  for (int i = 0; i < kColumns * kRows; ++i) {
    ImGui::PushID(i);

    if (i && (i % kColumns) == 0)
      ImGui::SetCursorPosX(startX);

    ImGui::ImageButton("##icon", (ImTextureID)m_textures[i],
                       {kButtonSize, kButtonSize}, {0.f, 0.f}, {1.f, 1.f},
                       ImVec4(0, 0, 0, 0));

    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();

    const bool active = (m_activeTool == kButtons[i].type);

    if (ImGui::IsItemClicked())
      m_activeTool = kButtons[i].type;

    if (ImGui::IsItemActive() || active)
      sunkenBorder(dl, min, max, 1.5f);
    else
      raisedBorder(dl, min, max, 1.5f);

    ImGui::PopID();

    if ((i + 1) % kColumns != 0)
      ImGui::SameLine();
  }

  renderOptions(editor, dl);

  ImVec2 winMin = ImGui::GetWindowPos();
  ImVec2 winMax = {winMin.x + ImGui::GetWindowWidth(),
                   winMin.y + ImGui::GetWindowHeight()};

  ImGui::End();

  raisedBorder(dl, winMin, winMax);

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
}

} // namespace UI
