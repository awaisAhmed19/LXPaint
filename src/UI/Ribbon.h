#pragma once

#include "imgui.h"
#include "UI/Dropdown/Dropdown.h"
#include "UI/Actions/MenuActionDispatcher.h"

#include <SDL3/SDL_rect.h>
#include <vector>

class Editor;

namespace UI {

// ─────────────────────────────────────────────────────────────────────────────
//  Ribbon
//
//  Owns all Dropdown instances and the MenuActionDispatcher.
//  Renders the horizontal menu bar and delegates popup rendering to each
//  Dropdown.  Dispatches any clicked MenuAction through the dispatcher.
//
//  Ribbon never calls Editor methods directly; MenuActionDispatcher does.
// ─────────────────────────────────────────────────────────────────────────────

class Ribbon {
public:
    Ribbon(int w, int h);

    // Must be called once after construction so the menu data is built.
    // Separated from the constructor so it can be deferred / re-called if
    // the Editor reference becomes available after construction.
    void buildMenus();
    float preferredHeight() const;
    void render(Editor& editor);

private:
    // ── Menu builders — pure data, no Editor refs ─────────────────────────
    static std::vector<MenuItem> buildFileMenu();
    static std::vector<MenuItem> buildEditMenu();
    static std::vector<MenuItem> buildViewMenu();
    static std::vector<MenuItem> buildImageMenu();
    static std::vector<MenuItem> buildColorsMenu();

    // ── Win95 border helpers ──────────────────────────────────────────────
    void raisedBorder(ImDrawList* dl, ImVec2 min, ImVec2 max,
                      float thickness = 1.f);
    void sunkenBorder(ImDrawList* dl, ImVec2 min, ImVec2 max,
                      float thickness = 1.f);

    void layout(const ImGuiViewport* vp);

    // ── State ─────────────────────────────────────────────────────────────
    const int   m_x = 0;
    const int   m_y = 0;
    int         m_w = 0;
    int         m_h = 0;
    SDL_FRect   m_rect{};
    ImU32       m_col = IM_COL32(192, 192, 192, 255);

    // Only one dropdown may be open at a time; this index tracks which one.
    int                   m_activeDropdown = -1;

    std::vector<Dropdown> m_dropdowns;
};

} // namespace UI
