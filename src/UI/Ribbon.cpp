#include "Ribbon.h"
#include "FooterMessages.h"
#include "Systems/Logger.h"
#include "UI/LayoutEngine/UILayoutConstant.h"
#include "imgui.h"
#include <array>

#include "Editor/Editor.h"

namespace UI {

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 RibbonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

Ribbon::Ribbon(int w, int h) : m_w(w), m_h(h) { buildMenus(); }

std::vector<MenuItem> Ribbon::buildFileMenu() {
  using MI = MenuItem;
  namespace K = FooterMessages::Key;
  return {
      MI::normal("New", MenuAction::FileNew, "Ctrl+Alt+N", true, K::FileNew),
      MI::normal("Open", MenuAction::FileOpen, "Ctrl+O", true, K::FileOpen),
      MI::normal("Save", MenuAction::FileSave, {}, true, K::FileSave),
      MI::normal("Save As", MenuAction::FileSaveAs, "Ctrl+Shift+S", true,
                 K::FileSaveAs),
      MI::separator(),
      MI::normal("Load From URL", MenuAction::FileLoadURL, {}, true,
                 K::FileLoadURL),
      MI::normal("Upload To Imgur", MenuAction::FileUploadImgur, {}, true,
                 K::FileUploadImgur),
      MI::separator(),
      MI::normal("Manage Storage", MenuAction::FileManageStorage, {}, true,
                 K::FileManageStorage),
      MI::separator(),
      MI::normal("Print Preview", MenuAction::FilePrintPreview, {}),
      MI::normal("Page Setup", MenuAction::FilePageSetup, {}),
      MI::normal("Print", MenuAction::FilePrint, "Ctrl+P"),
      MI::separator(),
      MI::normal("Set As Wallpaper (Tiled)", MenuAction::FileWallpaperTiled, {},
                 true, K::FileWallpaperTiled),
      MI::normal("Set As Wallpaper (Centered)",
                 MenuAction::FileWallpaperCentered, {}, true,
                 K::FileWallpaperCentr),
      MI::separator(),
      MI::normal("Recent File", MenuAction::None, {}, false,
                 K::FileRecentFiles),
      MI::separator(),
      MI::normal("Exit", MenuAction::FileExit, {}, true, K::FileExit),
  };
}

std::vector<MenuItem> Ribbon::buildEditMenu() {
  using MI = MenuItem;
  namespace K = FooterMessages::Key;
  return {
      MI::normal("Undo", MenuAction::EditUndo, "Ctrl+Z", true, K::EditUndo),
      MI::normal("Repeat", MenuAction::EditRedo, "F4", true, K::EditRedo),
      MI::normal("History", MenuAction::EditHistory, "Ctrl+Shift+Y", true,
                 K::EditHistory),
      MI::separator(),
      MI::normal("Cut", MenuAction::EditCut, "Ctrl+X", false, K::EditCut),
      MI::normal("Copy", MenuAction::EditCopy, "Ctrl+C", false, K::EditCopy),
      MI::normal("Paste", MenuAction::EditPaste, "Ctrl+V", true, K::EditPaste),
      MI::normal("Clear Selection", MenuAction::EditClearSelection, "Del",
                 false, K::EditClearSelection),
      MI::normal("Select All", MenuAction::EditSelectAll, "Ctrl+A", true,
                 K::EditSelectAll),
      MI::separator(),
      MI::normal("Copy To...", MenuAction::EditCopyTo, {}, true, K::EditCopyTo),
      MI::normal("Paste From...", MenuAction::EditPasteFrom, {}, true,
                 K::EditPasteFrom),
  };
}

std::vector<MenuItem> Ribbon::buildViewMenu() {
  using MI = MenuItem;
  namespace K = FooterMessages::Key;
  return {
      MI::checkbox("Tool Box", MenuAction::ViewToggleToolbox, true, true,
                   K::ViewToolbox),
      MI::checkbox("Color Box", MenuAction::ViewToggleColorBox, true, true,
                   K::ViewColorBox),
      MI::checkbox("Status Bar", MenuAction::ViewToggleStatusBar, true, true,
                   K::ViewStatusBar),
      MI::normal("Text Toolbar", MenuAction::ViewTextToolbar, {}, false,
                 K::ViewTextBar),
      MI::separator(),
      MI::normal("Zoom", MenuAction::ViewZoom, {}, true, K::ViewZoom),
      MI::normal("View Bitmap", MenuAction::ViewBitmap, "Ctrl+F", true,
                 K::ViewBitmap),
      MI::separator(),
      MI::normal("Fullscreen", MenuAction::ViewFullscreen, "F11", true,
                 K::ViewFullscreen),
  };
}

std::vector<MenuItem> Ribbon::buildImageMenu() {
  using MI = MenuItem;
  namespace K = FooterMessages::Key;
  return {
      MI::normal("Flip/Rotate", MenuAction::ImageFlipRotate, "Ctrl+Alt+R", true,
                 K::ImageFlipRotate),
      MI::normal("Stretch/Skew", MenuAction::ImageStretchSkew, "Ctrl+Alt+W",
                 true, K::ImageStretchSkew),
      MI::separator(),
      MI::normal("Invert Colors", MenuAction::ImageInvertColors, "Ctrl+I", true,
                 K::ImageInvert),
      MI::normal("Attributes...", MenuAction::ImageAttributes, "Ctrl+E", true,
                 K::ImageAttributes),
      MI::normal("Clear Image", MenuAction::ImageClear, "Ctrl+Shift+N", true,
                 K::ImageClear),
      MI::checkbox("Draw Opaque", MenuAction::ImageDrawOpaque, true, true,
                   K::ImageDrawOpaque),
  };
}

std::vector<MenuItem> Ribbon::buildColorsMenu() {
  using MI = MenuItem;
  namespace K = FooterMessages::Key;
  return {
      MI::normal("Edit Colors...", MenuAction::ColorsEdit, {}, true,
                 K::ColorsEdit),
      MI::normal("Get Colors...", MenuAction::ColorsGet, {}, true,
                 K::ColorsGet),
      MI::normal("Save Colors", MenuAction::ColorsSave, {}, true,
                 K::ColorsSave),
  };
}

void Ribbon::buildMenus() {
  m_dropdowns.clear();
  m_dropdowns.emplace_back("File", buildFileMenu());
  m_dropdowns.emplace_back("Edit", buildEditMenu());
  m_dropdowns.emplace_back("View", buildViewMenu());
  m_dropdowns.emplace_back("Colors", buildColorsMenu());
  m_dropdowns.emplace_back("Image", buildImageMenu());
  m_dropdowns.emplace_back("Help", std::vector<MenuItem>{});
  Logger::debug("Building ribbon menus");
}

void Ribbon::raisedBorder(ImDrawList *dl, ImVec2 min, ImVec2 max, float) {
  dl->AddLine(min, {max.x, min.y}, Theme::WHITE, 1.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::WHITE, 1.0f);
  dl->AddLine({min.x, max.y}, max, Theme::BLACK, 1.0f);
  dl->AddLine({max.x, min.y}, max, Theme::BLACK, 1.0f);
}

void Ribbon::sunkenBorder(ImDrawList *dl, ImVec2 min, ImVec2 max, float) {
  dl->AddLine(min, {max.x, min.y}, Theme::BLACK, 1.0f);
  dl->AddLine(min, {min.x, max.y}, Theme::BLACK, 1.0f);
  dl->AddLine({min.x, max.y}, max, Theme::WHITE, 1.0f);
  dl->AddLine({max.x, min.y}, max, Theme::WHITE, 1.0f);
}

void Ribbon::layout(const ImGuiViewport *vp) {
  constexpr float kRibbonHeight = 21.f;
  m_rect = {vp->Pos.x, vp->Pos.y, vp->Size.x, kRibbonHeight};
}

float Ribbon::preferredHeight() const { return UI::Layout::RibbonHeight; }

void Ribbon::render(Editor &editor) {
  constexpr float kBorderThickness = 1.f;
  constexpr float kRibbonButtonHeight = 21.f;
  constexpr float kButtonPadX = 10.f;
  constexpr ImVec2 kFramePadding{10.f, 2.f};
  constexpr ImVec2 kWindowPadding{0.f, 0.f};
  constexpr ImVec2 kItemSpacing{2.f, 0.f};

  constexpr ImGuiWindowFlags kWindowFlags =
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGuiViewport *vp = ImGui::GetMainViewport();
  layout(vp);

  ImGui::SetNextWindowPos({m_rect.x, m_rect.y}, ImGuiCond_Always);
  ImGui::SetNextWindowSize({m_rect.w, m_rect.h}, ImGuiCond_Always);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, kWindowPadding);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, kItemSpacing);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, kFramePadding);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, Theme::RibbonBg);
  ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextColor);
  ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

  ImGui::Begin("Ribbon", nullptr, kWindowFlags);
  ImDrawList *dl = ImGui::GetWindowDrawList();

  if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    for (auto &d : m_dropdowns)
      d.close();
    m_activeDropdown = -1;
  }

  const ImVec2 mousePos = ImGui::GetMousePos();

  // Sync View menu checkbox states
  if (m_dropdowns.size() > 2) {
    m_dropdowns[2].setChecked(0, editor.isToolboxVisible());
    m_dropdowns[2].setChecked(1, editor.isPaletteVisible());
    m_dropdowns[2].setChecked(2, editor.isStatusBarVisible());
  }

  float cursorX = m_rect.x + 4.f;

  for (int i = 0; i < static_cast<int>(m_dropdowns.size()); ++i) {
    Dropdown &d = m_dropdowns[i];

    const ImVec2 labelSize = ImGui::CalcTextSize(d.title().c_str());
    const float btnW = labelSize.x + kButtonPadX * 2.f;
    const float btnH = kRibbonButtonHeight;

    ImVec2 btnMin = {cursorX, m_rect.y};
    ImVec2 btnMax = {cursorX + btnW, m_rect.y + btnH};

    const bool isActive = (m_activeDropdown == i);
    const bool clicked = d.renderRibbonButton(dl, btnMin, btnMax, isActive);

    const bool hovered = mousePos.x >= btnMin.x && mousePos.x < btnMax.x &&
                         mousePos.y >= btnMin.y && mousePos.y < btnMax.y;

    if (clicked) {
      if (m_activeDropdown >= 0 && m_activeDropdown != i)
        m_dropdowns[m_activeDropdown].close();
      if (isActive) {
        d.close();
        m_activeDropdown = -1;
      } else {
        d.open();
        m_activeDropdown = i;
      }
    } else if (hovered && m_activeDropdown >= 0 && m_activeDropdown != i) {
      m_dropdowns[m_activeDropdown].close();
      d.open();
      m_activeDropdown = i;
    }

    cursorX += btnW + kItemSpacing.x;
  }

  const ImVec2 winMin = ImGui::GetWindowPos();
  const ImVec2 winMax = {winMin.x + ImGui::GetWindowWidth(),
                         winMin.y + ImGui::GetWindowHeight()};

  ImGui::End();
  raisedBorder(dl, winMin, winMax, kBorderThickness);

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);

  // ── Render open dropdown panel ────────────────────────────────────────────

  if (m_activeDropdown >= 0) {
    Dropdown &d = m_dropdowns[m_activeDropdown];

    float cx = m_rect.x + 4.f;
    for (int j = 0; j < m_activeDropdown; ++j)
      cx += ImGui::CalcTextSize(m_dropdowns[j].title().c_str()).x +
            kButtonPadX * 2.f + kItemSpacing.x;

    ImVec2 panelOrigin = {cx, m_rect.y + kRibbonButtonHeight};

    MenuAction action = d.renderPanel(panelOrigin);

    if (action != MenuAction::None) {
      MenuActionDispatcher::execute(action, editor);
      d.close();
      m_activeDropdown = -1;
      return;
    }

    // Close on outside click
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      bool insideRibbon = false;
      float buttonX = m_rect.x + 4.f;
      for (size_t i = 0; i < m_dropdowns.size(); ++i) {
        float bw = ImGui::CalcTextSize(m_dropdowns[i].title().c_str()).x +
                   kButtonPadX * 2.f;
        ImVec2 min = {buttonX, m_rect.y};
        ImVec2 max = {buttonX + bw, m_rect.y + kRibbonButtonHeight};
        if (mousePos.x >= min.x && mousePos.x <= max.x && mousePos.y >= min.y &&
            mousePos.y <= max.y) {
          insideRibbon = true;
          break;
        }
        buttonX += bw + kItemSpacing.x;
      }

      bool insidePanel = d.contains(panelOrigin, mousePos);
      if (!insideRibbon && !insidePanel) {
        d.close();
        m_activeDropdown = -1;
      }
    }
  }
}

} // namespace UI
