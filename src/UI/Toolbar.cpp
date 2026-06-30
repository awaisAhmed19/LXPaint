#include "Toolbar.h"
#include "App/Globals.h"
#include "BorderRenderer.h"
#include "Core/AssetManager.h"
#include "FooterMessages.h"
#include "HoverStatus.h"
#include "RetroWindow.h"
#include "Theme.h"
#include "imgui.h"
#include <SDL3/SDL_render.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

namespace UI {

struct ToolButton {
  ToolType type;
  const char *iconName;
  const char *messageKey;
};

static constexpr ToolButton kButtons[] = {
    {ToolType::FreeSelect, "01_free_form_select",
     FooterMessages::Key::FreeSelect},
    {ToolType::RectSelect, "02_select", FooterMessages::Key::RectSelect},
    {ToolType::Eraser, "03_eraser", FooterMessages::Key::Eraser},
    {ToolType::FloodFill, "04_fill_bucket", FooterMessages::Key::FloodFill},
    {ToolType::Eyedropper, "05_pick_color", FooterMessages::Key::Eyedropper},
    {ToolType::Magnifier, "06_magnifier", FooterMessages::Key::Magnifier},
    {ToolType::Pencil, "07_pencil", FooterMessages::Key::Pencil},
    {ToolType::Brush, "08_brush", FooterMessages::Key::Brush},
    {ToolType::Airbrush, "09_airbrush", FooterMessages::Key::Airbrush},
    {ToolType::Text, "10_text", FooterMessages::Key::Text},
    {ToolType::Line, "11_line", FooterMessages::Key::Line},
    {ToolType::Curve, "12_curve", FooterMessages::Key::Curve},
    {ToolType::Rectangle, "13_rectangle", FooterMessages::Key::Rectangle},
    {ToolType::Polygon, "14_polygon", FooterMessages::Key::Polygon},
    {ToolType::Ellipse, "15_ellipse", FooterMessages::Key::Ellipse},
    {ToolType::RoundedRectangle, "16_rounded_rectangle",
     FooterMessages::Key::RoundedRect},
};
static constexpr int TotalButtonsLocal = static_cast<int>(std::size(kButtons));

Toolbar::Toolbar(int w, int h, SDL_Renderer *renderer)
    : m_w(w), m_h(h), m_renderer(renderer) {}

Toolbar::~Toolbar() {
  for (int i = 0; i < TotalButtonsLocal; ++i) {
    if (m_textures[i]) {
      SDL_DestroyTexture(m_textures[i]);
      m_textures[i] = nullptr;
    }
  }
  if (m_backgroundTransparentIcon)
    SDL_DestroyTexture(m_backgroundTransparentIcon);
  if (m_backgroundOpaqueIcon)
    SDL_DestroyTexture(m_backgroundOpaqueIcon);
  m_renderer = nullptr;
}

bool Toolbar::init() {
  bool ok = true;
  for (int i = 0; i < TotalButtonsLocal; ++i) {
    const std::string path =
        AssetManager::toolIcon(std::string(kButtons[i].iconName) + ".png")
            .string();
    m_textures[i] = IMG_LoadTexture(m_renderer, path.c_str());
    if (!m_textures[i])
      ok = false;
  }
  {
    const std::string path =
        AssetManager::toolIcon("options-transparency-top.png").string();
    m_backgroundOpaqueIcon = IMG_LoadTexture(m_renderer, path.c_str());
    if (!m_backgroundOpaqueIcon)
      ok = false;
  }
  {
    const std::string path =
        AssetManager::toolIcon("options-transparency-bottom.png").string();
    m_backgroundTransparentIcon = IMG_LoadTexture(m_renderer, path.c_str());
    if (!m_backgroundTransparentIcon)
      ok = false;
  }
  return ok;
}
// ─── Option panels ───────────────────────────────────────────────────────

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

    float half =
        std::clamp(std::min(cellH, cellW) * 0.04f * sizes[i], 2.0f, 10.0f);
    dl->AddRectFilled({center.x - half, center.y - half},
                      {center.x + half, center.y + half},
                      selected ? Theme::White : Theme::Black);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 900);
    if (ImGui::InvisibleButton("##erasersize", {cellW, cellH}))
      target = static_cast<float>(sizes[i]);
    if (ImGui::IsItemHovered())
      HoverStatus::push(FooterMessages::Key::Eraser);
    ImGui::PopID();
  }
}

void Toolbar::renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                float optionWidth, float optionHeight) {
  constexpr float gap = 2.0f;
  constexpr float padding = 2.f;
  auto &brushShape = editor.getToolSettings().brushShape;
  auto &strokeWidth = editor.getToolSettings().strokeWidth;

  const float cellW = ((optionWidth - padding) - gap * 2.0f) / 3.0f;
  const float cellH = ((optionHeight - padding) - gap * 3.0f) / 4.0f;

  for (int i = 0; i < 12; ++i) {
    const int row = i / 3;
    const int col = i % 3;

    ImVec2 btnMin = {origin.x + padding + col * cellW,
                     origin.y + padding + row * cellH};
    ImVec2 btnMax = {btnMin.x + cellW, btnMin.y + cellH};

    float radius, thickness, widthValue;
    switch (col) {
    case 0:
      radius = 2.f;
      thickness = 1.f;
      widthValue = 1.f;
      break;
    case 1:
      radius = 4.f;
      thickness = 2.f;
      widthValue = 2.f;
      break;
    default:
      radius = 6.f;
      thickness = 3.f;
      widthValue = 3.f;
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
    case 0:
      dl->AddCircleFilled(center, radius,
                          selected ? Theme::White : Theme::Black);
      break;
    case 1:
      dl->AddRectFilled({center.x - radius, center.y - radius},
                        {center.x + radius, center.y + radius},
                        selected ? Theme::White : Theme::Black);
      break;
    case 2:
      dl->AddLine({center.x - radius, center.y + radius},
                  {center.x + radius, center.y - radius},
                  selected ? Theme::White : Theme::Black, thickness);
      break;
    case 3:
      dl->AddLine({center.x - radius, center.y - radius},
                  {center.x + radius, center.y + radius},
                  selected ? Theme::White : Theme::Black, thickness);
      break;
    }

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 300);
    if (ImGui::InvisibleButton("##brush", {cellW, cellH})) {
      brushShape = static_cast<ToolSettings::BrushShape>(row);
      strokeWidth = widthValue;
    }
    if (ImGui::IsItemHovered())
      HoverStatus::push(FooterMessages::Key::Brush);
    ImGui::PopID();
  }
}

void Toolbar::renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                              float optionWidth, float optionHeight) {
  constexpr float gap = 1.0f;
  auto &mode = editor.getToolSettings().fillmode;

  struct FillPreview {
    const char *id;
    bool outline;
    bool filled;
  };
  static constexpr FillPreview kModes[3] = {
      {"##fm0", true, false},
      {"##fm1", false, true},
      {"##fm2", true, true},
  };

  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 2.0f) / 3.0f;

  const char *fillKey = FooterMessages::Key::Rectangle;

  for (int i = 0; i < 3; ++i) {
    ImVec2 btnMin = {origin.x + 1.f, origin.y + 1.f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 1.f, btnMin.y + cellH - 1.f};

    bool selected = (mode == static_cast<ToolSettings::FillMode>(i));
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    constexpr float padX = 8.f, padY = 4.f;
    ImVec2 rectMin = {btnMin.x + padX, btnMin.y + padY};
    ImVec2 rectMax = {btnMax.x - padX, btnMax.y - padY};
    if (kModes[i].filled)
      dl->AddRectFilled(rectMin, rectMax, IM_COL32(128, 128, 128, 255));
    if (kModes[i].outline)
      dl->AddRect(rectMin, rectMax, IM_COL32(128, 128, 128, 255));

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 400);
    if (ImGui::InvisibleButton(kModes[i].id, {cellW, cellH}))
      mode = static_cast<ToolSettings::FillMode>(i);
    if (ImGui::IsItemHovered())
      HoverStatus::push(fillKey);
    ImGui::PopID();
  }
}

void Toolbar::renderLineWidths(Editor &editor, ImDrawList *dl, ImVec2 origin,
                               float optionWidth, float optionHeight) {
  static constexpr float kWidths[4] = {1.f, 2.f, 3.f, 5.f};
  constexpr float gap = 1.f;
  float &target = editor.getToolSettings().lineWidth;
  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 3.f) / 4.f;

  for (int i = 0; i < 4; ++i) {
    ImVec2 btnMin = {origin.x + 1.f, origin.y + 1.f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 1.f, btnMin.y + cellH - 1.f};
    ImVec2 center = {(btnMin.x + btnMax.x) * 0.5f,
                     (btnMin.y + btnMax.y) * 0.5f};

    bool selected = (target == kWidths[i]);
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);
    dl->AddLine({btnMin.x + 6.f, center.y}, {btnMax.x - 6.f, center.y},
                selected ? Theme::White : Theme::Black, kWidths[i]);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 500);
    if (ImGui::InvisibleButton("##linewidth", {cellW, cellH}))
      target = kWidths[i];
    if (ImGui::IsItemHovered())
      HoverStatus::push(FooterMessages::Key::Line);
    ImGui::PopID();
  }
}

void Toolbar::renderAirbrushSizes(Editor &editor, ImDrawList *dl, ImVec2 origin,
                                  float optionWidth, float optionHeight) {
  static constexpr float kSizes[3] = {5.f, 8.f, 12.f};
  static constexpr int kDotCounts[3] = {10, 18, 28};
  const float gap = 2.f;
  const float cellW = optionWidth;
  const float cellH = (optionHeight - gap * 3.f) / 3.f;
  float &target = editor.getToolSettings().airbrushRadius;

  for (int i = 0; i < 3; ++i) {
    ImVec2 btnMin = {origin.x + 1.f, origin.y + 1.f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 1.f, btnMin.y + cellH - 1.f};
    bool selected = (target == kSizes[i]);
    ImVec2 c = {(btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f};

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    float radius = kSizes[i] * 0.6f;
    unsigned seed = 1234u + (unsigned)i * 97u;
    auto rnd = [&seed]() {
      seed = seed * 1103515245u + 12345u;
      return (float)((seed >> 16) & 0x7FFF) / 32767.f;
    };
    for (int d = 0; d < kDotCounts[i]; ++d) {
      float ang = rnd() * 6.2831853f;
      float r = rnd() * radius;
      dl->AddCircleFilled({c.x + std::cos(ang) * r, c.y + std::sin(ang) * r},
                          1.f, selected ? Theme::White : Theme::Black);
    }

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 600);
    if (ImGui::InvisibleButton("##spray", {cellW, cellH}))
      target = kSizes[i];
    if (ImGui::IsItemHovered())
      HoverStatus::push(FooterMessages::Key::Airbrush);
    ImGui::PopID();
  }
}

void Toolbar::renderZoomLevels(Editor &editor, ImDrawList *dl, ImVec2 origin,
                               float optionWidth, float optionHeight) {
  static constexpr int kLevels[4] = {1, 2, 6, 8};
  constexpr float gap = 2.f;
  const float cellW = optionWidth;
  const float cellH = optionHeight / 4.f;
  int &target = editor.getToolSettings().zoomLevel;

  for (int i = 0; i < 4; ++i) {
    ImVec2 btnMin = {origin.x + 1.f, origin.y + 1.f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 1.f, btnMin.y + cellH - 1.f};
    bool selected = (target == kLevels[i]);

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    char label[8];
    std::snprintf(label, sizeof(label), "%dx", kLevels[i]);
    ImVec2 textSize = ImGui::CalcTextSize(label);
    ImVec2 textPos = {(btnMin.x + btnMax.x - textSize.x) * 0.5f,
                      (btnMin.y + btnMax.y - textSize.y) * 0.5f};
    dl->AddText(textPos, selected ? Theme::White : Theme::Black, label);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 800);
    if (ImGui::InvisibleButton("##zoom", {cellW, cellH}))
      target = kLevels[i];
    if (ImGui::IsItemHovered())
      HoverStatus::push(FooterMessages::Key::Magnifier);
    ImGui::PopID();
  }
}

void Toolbar::renderBackgroundModeIcons(Editor &editor, ImDrawList *dl,
                                        ImVec2 origin, float optionWidth,
                                        float optionHeight,
                                        ToolSettings::BackgroundMode &target) {
  constexpr float gap = 0.f;
  const float cellW = optionWidth;
  const float cellH = optionHeight / 2.f;

  ImTextureID icons[2] = {
      (ImTextureID)m_backgroundOpaqueIcon,
      (ImTextureID)m_backgroundTransparentIcon,
  };

  const char *key = FooterMessages::Key::RectSelect;

  for (int i = 0; i < 2; ++i) {
    ToolSettings::BackgroundMode mode =
        (i == 0) ? ToolSettings::BackgroundMode::Opaque
                 : ToolSettings::BackgroundMode::Transparent;

    ImVec2 btnMin = {origin.x + 1.f, origin.y + 1.f + i * (cellH + gap)};
    ImVec2 btnMax = {btnMin.x + cellW - 3.f, btnMin.y + cellH - 3.f};
    bool selected = (target == mode);

    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      dl->AddRectFilled(btnMin, btnMax, Theme::OptionHovered);

    constexpr float ipad = 5.f;
    ImGui::SetCursorScreenPos({btnMin.x, btnMin.y});
    ImGui::PushID(i + 700);
    ImGui::ImageButton("##bgmode", icons[i],
                       {cellW - 2.f * ipad, cellH - 2.f * ipad}, {0.f, 0.f},
                       {1.f, 1.f}, ImVec4(0, 0, 0, 0));
    if (ImGui::IsItemClicked())
      target = mode;
    if (ImGui::IsItemHovered())
      HoverStatus::push(key);
    ImGui::PopID();
  }
}
// ─── Options dispatcher — now positioned via the ToolbarMetrics-derived box
void Toolbar::renderOptions(Editor &editor, ImDrawList *dl,
                            const PanelRect &box) {
  ImVec2 boxMin = {box.x, box.y};
  ImVec2 boxMax = {box.x + box.width, box.y + box.height};
  dl->AddRectFilled(boxMin, boxMax, Theme::WindowBg);
  BorderRenderer::Sunken(dl, boxMin, boxMax);

  static constexpr int kEraserSizes[] = {4, 6, 8, 10};

  switch (editor.getActiveTool()) {
  case ToolType::Eraser:
    renderSizeSquares(editor, dl, boxMin, box.width, box.height, kEraserSizes,
                      4);
    break;
  case ToolType::Line:
  case ToolType::Curve:
    renderLineWidths(editor, dl, boxMin, box.width, box.height);
    break;
  case ToolType::Airbrush:
    renderAirbrushSizes(editor, dl, boxMin, box.width, box.height);
    break;
  case ToolType::Brush:
    renderBrushShapes(editor, dl, boxMin, box.width, box.height);
    break;
  case ToolType::Rectangle:
  case ToolType::Ellipse:
  case ToolType::RoundedRectangle:
  case ToolType::Polygon:
    renderFillModes(editor, dl, boxMin, box.width, box.height);
    break;
  case ToolType::FreeSelect:
  case ToolType::RectSelect:
  case ToolType::Text:
    renderBackgroundModeIcons(editor, dl, boxMin, box.width, box.height,
                              editor.getToolSettings().backgroundMode);
    break;
  case ToolType::Magnifier:
    renderZoomLevels(editor, dl, boxMin, box.width, box.width);
    break;
  default:
    break;
  }
}

// ─── Main render — fully driven by LayoutMetrics::toolbarMetrics ────────
void Toolbar::render(Editor &editor, const LayoutMetrics &layout) {
  const ToolbarMetrics &tm = layout.toolbarMetrics;
  const PanelRect &rect = layout.toolbar;

  RetroWindowDesc desc;
  desc.pos = {rect.x, rect.y + 4.f};
  desc.size = {rect.width + tm.leftInset, rect.height - 6.f};
  desc.itemSpacing = {tm.buttonGap, tm.buttonGap};
  desc.framePadding = {1.f, 1.f};

  RetroWindow win("ToolbarGrid", desc);
  ImDrawList *dl = win.drawList();

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + tm.leftInset);
  const float startX = ImGui::GetCursorPosX();

  for (int i = 0; i < tm.columns * tm.rows; ++i) {
    ImGui::PushID(i);

    if (i && (i % tm.columns) == 0)
      ImGui::SetCursorPosX(startX);

    if (i < TotalButtonsLocal) {
      ImGui::ImageButton("##icon", (ImTextureID)m_textures[i],
                         {tm.buttonSize, tm.buttonSize}, {0.f, 0.f}, {1.f, 1.f},
                         ImVec4(0, 0, 0, 0));

      ImVec2 min = ImGui::GetItemRectMin();
      ImVec2 max = ImGui::GetItemRectMax();

      const bool active = (m_activeTool == kButtons[i].type);

      if (ImGui::IsItemClicked())
        m_activeTool = kButtons[i].type;

      if (ImGui::IsItemHovered())
        HoverStatus::push(kButtons[i].messageKey);

      if (ImGui::IsItemActive() || active)
        BorderRenderer::Sunken(dl, min, max, 1.5f);
      else
        BorderRenderer::Raised(dl, min, max, 1.5f);
    } else {
      // Grid cell with no assigned tool (future buttons) — reserve the
      // space so layout stays stable, draw nothing.
      ImGui::Dummy({tm.buttonSize, tm.buttonSize});
    }

    ImGui::PopID();

    if ((i + 1) % tm.columns != 0)
      ImGui::SameLine();
  }

  renderOptions(editor, dl, tm.optionsBox);

  BorderRenderer::Raised(dl, win.min(), win.max());
}

} // namespace UI
