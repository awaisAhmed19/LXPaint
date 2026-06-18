#include "Toolbar.h"
#include <SDL3_image/SDL_image.h>
#include <algorithm>
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

struct ToolButton {
  ToolType type;
  const char *iconName;
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

Toolbar::Toolbar(int w, int h) : m_w(w), m_h(h) {}

Toolbar::~Toolbar() {
  for (int i = 0; i < TotalButtons; ++i) {
    if (m_textures[i]) {
      SDL_DestroyTexture(m_textures[i]);
      m_textures[i] = nullptr;
    }
  }
}

void Toolbar::raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, Theme::WHITE, 2.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::WHITE, 2.0f);
  dl->AddLine({min.x, max.y}, max, Theme::BLACK, 2.0f);
  dl->AddLine({max.x, min.y}, max, Theme::BLACK, 2.0f);
}

void Toolbar::sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max) {
  dl->AddLine(min, {max.x, min.y}, Theme::BLACK, 2.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::BLACK, 2.0f);
  dl->AddLine({min.x, max.y}, max, Theme::WHITE, 2.0f);
  dl->AddLine({max.x, min.y}, max, Theme::WHITE, 2.0f);
}

bool Toolbar::init(SDL_Renderer *renderer) {
  bool ok = true;
  for (int i = 0; i < TotalButtons; ++i) {
    std::string path =
        "../tools_icons/" + std::string(kButtons[i].iconName) + ".png";
    m_textures[i] = IMG_LoadTexture(renderer, path.c_str());
    if (!m_textures[i]) {
      SDL_Log("Toolbar: failed to load '%s': %s", path.c_str(), SDL_GetError());
      ok = false;
    }
  }
  return ok;
}

// Draws 4 dot-size options in a 2×2 grid, each a small square button
// with a filled circle whose radius reflects the size value.
void Toolbar::renderSizeDots(Editor &editor, ImDrawList *dl, ImVec2 origin,
                             const int *sizes, int count) {
  const float cell = 26.0f; // button cell size
  const float pad = 0.0f;

  int &current = editor.getToolSettings().brushSize;
  // For Line tool we want lineWidth instead
  ToolType t = editor.getActiveTool();
  int *target = (t == ToolType::Line) ? &editor.getToolSettings().lineWidth
                                      : &editor.getToolSettings().brushSize;

  for (int i = 0; i < count; ++i) {
    int col = i % 2;
    int row = i / 2;

    ImVec2 btnMin = {origin.x + col * (cell + pad),
                     origin.y + row * (cell + pad)};
    ImVec2 btnMax = {btnMin.x + cell, btnMin.y + cell};
    ImVec2 center = {(btnMin.x + btnMax.x) * 0.5f,
                     (btnMin.y + btnMax.y) * 0.5f};

    bool selected = (*target == sizes[i]);

    // Button background — sunken if selected
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      sunkenBorder(dl, btnMin, btnMax);
    else
      raisedBorder(dl, btnMin, btnMax);

    // Dot — radius scales with size, capped for display
    float radius = std::clamp(sizes[i] * 1.8f, 2.0f, 10.0f);
    dl->AddCircleFilled(center, radius, Theme::BLACK);

    // Invisible hit area
    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 200);
    if (ImGui::InvisibleButton("##dot", {cell, cell}))
      *target = sizes[i];
    ImGui::PopID();
  }
}

// 2×2 grid of brush-shape stamps (square / diagonal / round / spray)
void Toolbar::renderBrushShapes(Editor &editor, ImDrawList *dl, ImVec2 origin) {
  const float cell = 26.0f;
  const float pad = 2.0f;
  int &shape = editor.getToolSettings().brushShape;

  // 4 shapes: round, square, forward-slash, backslash
  for (int i = 0; i < 4; ++i) {
    int col = i % 2;
    int row = i / 2;
    ImVec2 btnMin = {origin.x + col * (cell + pad),
                     origin.y + row * (cell + pad)};
    ImVec2 btnMax = {btnMin.x + cell, btnMin.y + cell};

    bool selected = (shape == i);
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      sunkenBorder(dl, btnMin, btnMax);
    else
      raisedBorder(dl, btnMin, btnMax);

    ImVec2 c = {(btnMin.x + btnMax.x) * 0.5f, (btnMin.y + btnMax.y) * 0.5f};
    float r = 5.0f;

    switch (i) {
    case 0: // circle
      dl->AddCircleFilled(c, r, Theme::BLACK);
      break;
    case 1: // square
      dl->AddRectFilled({c.x - r, c.y - r}, {c.x + r, c.y + r}, Theme::BLACK);
      break;
    case 2: // forward-slash stroke
      dl->AddLine({c.x - r, c.y + r}, {c.x + r, c.y - r}, Theme::BLACK, 2.5f);
      break;
    case 3: // backslash stroke
      dl->AddLine({c.x - r, c.y - r}, {c.x + r, c.y + r}, Theme::BLACK, 2.5f);
      break;
    }

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 300);
    if (ImGui::InvisibleButton("##shape", {cell, cell}))
      shape = i;
    ImGui::PopID();
  }
}

// 3 fill-mode options stacked vertically — outline / fill / both
void Toolbar::renderFillModes(Editor &editor, ImDrawList *dl, ImVec2 origin) {
  const float w = 54.0f;
  const float h = 18.0f;
  const float gap = 2.0f;
  int &mode = editor.getToolSettings().brushShape; // reuse as fill mode

  struct FillPreview {
    const char *id;
    bool drawOutline;
    bool drawFill;
  };
  static constexpr FillPreview kModes[3] = {
      {"##fm0", true, false}, // outline only
      {"##fm1", false, true}, // fill only
      {"##fm2", true, true},  // outline + fill
  };

  for (int i = 0; i < 3; ++i) {
    ImVec2 btnMin = {origin.x, origin.y + i * (h + gap)};
    ImVec2 btnMax = {btnMin.x + w, btnMin.y + h};

    bool selected = (mode == i);
    dl->AddRectFilled(btnMin, btnMax, Theme::ButtonBg);
    if (selected)
      sunkenBorder(dl, btnMin, btnMax);
    else
      raisedBorder(dl, btnMin, btnMax);

    // Mini shape preview inside the button
    float px = btnMin.x + 6.0f, py = btnMin.y + 3.0f;
    float pw = w - 16.0f, phh = h - 6.0f;
    if (kModes[i].drawFill)
      dl->AddRectFilled({px, py}, {px + pw, py + phh},
                        IM_COL32(180, 180, 180, 255));
    if (kModes[i].drawOutline)
      dl->AddRect({px, py}, {px + pw, py + phh}, Theme::BLACK, 0.0f, 0, 1.5f);

    ImGui::SetCursorScreenPos(btnMin);
    ImGui::PushID(i + 400);
    if (ImGui::InvisibleButton(kModes[i].id, {w, h}))
      mode = i;
    ImGui::PopID();
  }
}

void Toolbar::renderOptions(Editor &editor, ImDrawList *dl) {
  // Options box starts below the 8-row button grid.
  // Button grid: 8 rows × 24px + gaps ≈ y offset ~220px from window top.
  // We draw the sunken container rect first, then dispatch per-tool content.

  ImVec2 winPos = ImGui::GetWindowPos();
  float boxX = winPos.x + 4.0f;
  float boxY = winPos.y + 222.0f; // just below the 16 buttons
  float boxW = 58.0f;
  float boxH = 80.0f;

  // Sunken container — classic Paint look
  ImVec2 boxMin = {boxX, boxY};
  ImVec2 boxMax = {boxX + boxW, boxY + boxH};
  dl->AddRectFilled(boxMin, boxMax, IM_COL32(192, 192, 192, 255));
  sunkenBorder(dl, boxMin, boxMax);

  // Inner content origin with a small inset
  ImVec2 inner = {boxX + 4.0f, boxY + 6.0f};

  static constexpr int kPencilSizes[] = {1, 2, 4, 6};
  static constexpr int kEraserSizes[] = {4, 6, 8, 10};
  static constexpr int kLineSizes[] = {1, 2, 3, 5};
  static constexpr int kAirSizes[] = {5, 8, 12, 18};

  switch (editor.getActiveTool()) {

  case ToolType::Pencil:
    renderSizeDots(editor, dl, inner, kPencilSizes, 4);
    break;

  case ToolType::Eraser:
    renderSizeDots(editor, dl, inner, kEraserSizes, 4);
    break;

  case ToolType::Line:
  case ToolType::Curve:
    renderSizeDots(editor, dl, inner, kLineSizes, 4);
    break;

  case ToolType::Airbrush:
    renderSizeDots(editor, dl, inner, kAirSizes, 4);
    break;

  case ToolType::Brush:
    renderBrushShapes(editor, dl, inner);
    break;

  case ToolType::Rectangle:
  case ToolType::Ellipse:
  case ToolType::RoundedRectangle:
  case ToolType::Polygon:
    renderFillModes(editor, dl, inner);
    break;

  default:
    // No options for select, eyedropper, magnifier, text, floodfill
    break;
  }
}

// ── main render ──────────────────────────────────────────────────────────────

void Toolbar::render(Editor &editor) {
  constexpr ImVec2 ButtonPadding = {1.0f, 1.0f};
  const float buttonSide = 24.0f;
  const int columns = 2;
  const int rows = 8;
  const int total = columns * rows;

  ImGuiViewport *vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos({vp->Pos.x, vp->Pos.y + 22.0f});
  ImGui::SetNextWindowSize({toolMax.x, toolMax.y});

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.0f, 0.0f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {2.0f, 0.0f});
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

  ImDrawList *dl = ImGui::GetWindowDrawList();

  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4.0f);
  float startX = ImGui::GetCursorPosX();

  ImVec2 buttonSize = {buttonSide, buttonSide};

  for (int i = 0; i < total; ++i) {
    ImGui::PushID(i);

    if ((i % columns) == 0 && i != 0)
      ImGui::SetCursorPosX(startX);

    ImTextureID tex = (ImTextureID)m_textures[i];
    ImGui::ImageButton("##icon", tex, buttonSize, {0.0f, 0.0f}, {1.0f, 1.0f},
                       ImVec4(0, 0, 0, 0));

    ImVec2 btnMin = ImGui::GetItemRectMin();
    ImVec2 btnMax = ImGui::GetItemRectMax();
    bool isActive = (m_activeTool == kButtons[i].type);

    if (ImGui::IsItemClicked()) {
      m_activeTool = kButtons[i].type;
      isActive = true;
    }

    if (ImGui::IsItemActive() || isActive)
      sunkenBorder(dl, btnMin, btnMax);
    else
      raisedBorder(dl, btnMin, btnMax);

    ImGui::PopID();

    if ((i + 1) % columns != 0)
      ImGui::SameLine();
  }

  renderOptions(editor, dl);

  ImGui::End();
  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);
  raisedBorder(dl, toolMin, toolMax);
}

} // namespace UI
