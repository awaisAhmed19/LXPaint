#include "Ribbon.h"
#include "Systems/Logger.h"
#include "UI/LayoutEngine/UILayoutConstant.h"
#include "imgui.h"
#include <array>

namespace UI {

// ─────────────────────────────────────────────────────────────────────────────
//  Local theme constants (Win95 palette, same as the rest of the UI)
// ─────────────────────────────────────────────────────────────────────────────

namespace Theme {
constexpr ImU32 BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 RibbonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonBg = IM_COL32(192, 192, 192, 255);
constexpr ImU32 ButtonHover = IM_COL32(210, 210, 210, 255);
constexpr ImU32 ButtonActive = IM_COL32(150, 150, 150, 255);
constexpr ImU32 TextColor = IM_COL32(0, 0, 0, 255);
} // namespace Theme

// ─────────────────────────────────────────────────────────────────────────────
//  Construction
// ─────────────────────────────────────────────────────────────────────────────

Ribbon::Ribbon(int w, int h) : m_w(w), m_h(h) {
  buildMenus(); // just added this now which started showing the menu buttons
                // but the buttons are not active for more than a sec and the
                // dropdown is just flickering once and deactivating
}

// ─────────────────────────────────────────────────────────────────────────────
//  Menu data — pure MenuItem trees, no Editor knowledge
// ─────────────────────────────────────────────────────────────────────────────

std::vector<MenuItem> Ribbon::buildFileMenu() {
  using MI = MenuItem;
  return {
      MI::normal("New", MenuAction::FileNew, "Ctrl+Alt+N"),
      MI::normal("Open", MenuAction::FileOpen, "Ctrl+O"),
      MI::normal("Save", MenuAction::FileSave),
      MI::normal("Save As", MenuAction::FileSaveAs, "Ctrl+Shift+S"),
      MI::separator(),
      MI::normal("Load From URL", MenuAction::FileLoadURL),
      MI::normal("Upload To Imgur", MenuAction::FileUploadImgur),
      MI::separator(),
      MI::normal("Manage Storage", MenuAction::FileManageStorage),
      MI::separator(),
      MI::normal("Print Preview", MenuAction::FilePrintPreview),
      MI::normal("Page Setup", MenuAction::FilePageSetup),
      MI::normal("Print", MenuAction::FilePrint, "Ctrl+P"),
      MI::separator(),
      MI::normal("Set As Wallpaper (Tiled)", MenuAction::FileWallpaperTiled),
      MI::normal("Set As Wallpaper (Centered)",
                 MenuAction::FileWallpaperCentered),
      MI::separator(),
      MI::normal("Recent File", MenuAction::None, {}, false),
      MI::separator(),
      MI::normal("Exit", MenuAction::FileExit),
  };
}

std::vector<MenuItem> Ribbon::buildEditMenu() {
  using MI = MenuItem;
  return {
      // Undo/Redo are now wired through to CommandManager via
      // Editor::undo()/Editor::redo() (MenuActionDispatcher::doUndo/doRedo),
      // so these are enabled — previously they were left disabled (false)
      // because nothing backed them yet; that's no longer the case.
      MI::normal("Undo", MenuAction::EditUndo, "Ctrl+Z", true),
      MI::normal("Repeat", MenuAction::EditRedo, "F4", true),
      MI::normal("History", MenuAction::EditHistory, "Ctrl+Shift+Y"),
      MI::separator(),
      MI::normal("Cut", MenuAction::EditCut, "Ctrl+X", false),
      MI::normal("Copy", MenuAction::EditCopy, "Ctrl+C", false),
      MI::normal("Paste", MenuAction::EditPaste, "Ctrl+V"),
      MI::normal("Clear Selection", MenuAction::EditClearSelection, "Del",
                 false),
      MI::normal("Select All", MenuAction::EditSelectAll, "Ctrl+A"),
      MI::separator(),
      MI::normal("Copy To...", MenuAction::EditCopyTo),
      MI::normal("Paste From...", MenuAction::EditPasteFrom),
  };
}

std::vector<MenuItem> Ribbon::buildViewMenu() {
  using MI = MenuItem;
  return {
      MI::checkbox("Tool Box", MenuAction::ViewToggleToolbox, true),
      MI::checkbox("Color Box", MenuAction::ViewToggleColorBox, true, true),
      MI::checkbox("Status Bar", MenuAction::ViewToggleStatusBar, true),
      MI::normal("Text Toolbar", MenuAction::ViewTextToolbar, {}, false),
      MI::separator(),
      MI::normal("Zoom", MenuAction::ViewZoom),
      MI::normal("View Bitmap", MenuAction::ViewBitmap, "Ctrl+F"),
      MI::separator(),
      MI::normal("Fullscreen", MenuAction::ViewFullscreen, "F11"),
  };
}

std::vector<MenuItem> Ribbon::buildImageMenu() {
  using MI = MenuItem;
  return {
      MI::normal("Flip/Rotate", MenuAction::ImageFlipRotate, "Ctrl+Alt+R"),
      MI::normal("Stretch/Skew", MenuAction::ImageStretchSkew, "Ctrl+Alt+W"),
      MI::separator(),
      MI::normal("Invert Colors", MenuAction::ImageInvertColors, "Ctrl+I"),
      MI::normal("Attributes...", MenuAction::ImageAttributes, "Ctrl+E"),
      MI::normal("Clear Image", MenuAction::ImageClear, "Ctrl+Shift+N"),
      MI::checkbox("Draw Opaque", MenuAction::ImageDrawOpaque, true),
  };
}

std::vector<MenuItem> Ribbon::buildColorsMenu() {
  using MI = MenuItem;
  return {
      MI::normal("Edit Colors...", MenuAction::ColorsEdit),
      MI::normal("Get Colors...", MenuAction::ColorsGet),
      MI::normal("Save Colors", MenuAction::ColorsSave),
  };
}

// ─────────────────────────────────────────────────────────────────────────────
//  buildMenus — assembles Dropdown objects from the data above
// ─────────────────────────────────────────────────────────────────────────────

void Ribbon::buildMenus() {
  m_dropdowns.clear();
  m_dropdowns.emplace_back("File", buildFileMenu());
  m_dropdowns.emplace_back("Edit", buildEditMenu());
  m_dropdowns.emplace_back("View", buildViewMenu());
  m_dropdowns.emplace_back("Colors", buildColorsMenu());
  m_dropdowns.emplace_back("Image", buildImageMenu());
  m_dropdowns.emplace_back("Help", std::vector<MenuItem>{}); // placeholder
  Logger::debug("Building ribbon menus");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Border helpers
// ─────────────────────────────────────────────────────────────────────────────

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

// ─────────────────────────────────────────────────────────────────────────────
//  Layout
// ─────────────────────────────────────────────────────────────────────────────

void Ribbon::layout(const ImGuiViewport *vp) {
  constexpr float kRibbonHeight = 21.0f;
  m_rect = {vp->Pos.x, vp->Pos.y, vp->Size.x, kRibbonHeight};
}

float Ribbon::preferredHeight() const { return UI::Layout::RibbonHeight; }

// ─────────────────────────────────────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────────────────────────────────────

void Ribbon::render(Editor &editor) {
  constexpr float kBorderThickness = 1.0f;
  constexpr float kRibbonButtonHeight = 21.0f;
  constexpr float kButtonPadX = 10.0f; // horizontal padding per button

  constexpr ImVec2 kFramePadding{10.0f, 2.0f};
  constexpr ImVec2 kWindowPadding{0.0f, 0.0f};
  constexpr ImVec2 kItemSpacing{2.0f, 0.0f};

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

  // Escape pressed: close all menus
  if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    for (auto &d : m_dropdowns)
      d.close();
    m_activeDropdown = -1;
  }

  // Close all if a click happened outside any open menu
  // (individual dropdowns also self-close; this is a belt-and-suspenders
  //  backstop for the ribbon button row itself)
  const ImVec2 mousePos = ImGui::GetMousePos();

  float cursorX = m_rect.x + 4.0f; // small left inset

  for (int i = 0; i < static_cast<int>(m_dropdowns.size()); ++i) {
    Dropdown &d = m_dropdowns[i];

    // Compute button bounds
    const ImVec2 labelSize = ImGui::CalcTextSize(d.title().c_str());
    const float btnW = labelSize.x + kButtonPadX * 2.0f;
    const float btnH = kRibbonButtonHeight;

    ImVec2 btnMin = {cursorX, m_rect.y};
    ImVec2 btnMax = {cursorX + btnW, m_rect.y + btnH};

    const bool isActive = (m_activeDropdown == i);

    // Let the Dropdown render its own button and tell us if it was clicked
    const bool clicked = d.renderRibbonButton(dl, btnMin, btnMax, isActive);

    // Mouse-hover while another menu is open: switch menus immediately
    const bool hovered = mousePos.x >= btnMin.x && mousePos.x < btnMax.x &&
                         mousePos.y >= btnMin.y && mousePos.y < btnMax.y;

    if (clicked) {
      // Close any previously-open menu
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
      // Hover-switch while a menu is already open (classic Win95 feel)
      m_dropdowns[m_activeDropdown].close();
      d.open();
      m_activeDropdown = i;
    }

    cursorX += btnW + kItemSpacing.x;
  }

  // ── Window border ──────────────────────────────────────────────────────

  const ImVec2 winMin = ImGui::GetWindowPos();
  const ImVec2 winMax = {winMin.x + ImGui::GetWindowWidth(),
                         winMin.y + ImGui::GetWindowHeight()};

  ImGui::End();

  raisedBorder(dl, winMin, winMax, kBorderThickness);

  ImGui::PopStyleColor(5);
  ImGui::PopStyleVar(4);

  // ── Render open dropdown panels (outside the Ribbon ImGui window) ──────
  // ────────────────────────────────────────────────────────────────────────
  // Render active dropdown
  // ────────────────────────────────────────────────────────────────────────

  if (m_activeDropdown >= 0) {
    Dropdown &d = m_dropdowns[m_activeDropdown];

    float cx = m_rect.x + 4.0f;
    for (int j = 0; j < m_activeDropdown; ++j) {
      cx += ImGui::CalcTextSize(m_dropdowns[j].title().c_str()).x +
            kButtonPadX * 2.0f + kItemSpacing.x;
    }

    ImVec2 panelOrigin = {
        cx,
        m_rect.y + kRibbonButtonHeight,
    };

    MenuAction action = d.renderPanel(panelOrigin);

    // Execute selected menu item
    if (action != MenuAction::None) {
      MenuActionDispatcher::execute(action, editor);

      d.close();
      m_activeDropdown = -1;
      return;
    }

    // Ignore the opening click.
    // Only close if the user clicked outside BOTH the ribbon buttons
    // and the dropdown itself.
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {

      bool insideRibbon = false;

      float buttonX = m_rect.x + 4.0f;

      for (size_t i = 0; i < m_dropdowns.size(); ++i) {

        float buttonWidth =
            ImGui::CalcTextSize(m_dropdowns[i].title().c_str()).x +
            kButtonPadX * 2.0f;

        ImVec2 min = {
            buttonX,
            m_rect.y,
        };

        ImVec2 max = {
            buttonX + buttonWidth,
            m_rect.y + kRibbonButtonHeight,
        };

        if (mousePos.x >= min.x && mousePos.x <= max.x && mousePos.y >= min.y &&
            mousePos.y <= max.y) {
          insideRibbon = true;
          break;
        }

        buttonX += buttonWidth + kItemSpacing.x;
      }

      bool insidePanel = d.contains(panelOrigin, mousePos);

      if (!insideRibbon && !insidePanel) {
        d.close();
        m_activeDropdown = -1;
      }
    }
  }
}
}; // namespace UI
