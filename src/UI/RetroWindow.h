#pragma once
#include "Theme.h"
#include "imgui.h"

namespace UI {

struct RetroWindowDesc {
  ImVec2 pos{0.f, 0.f};
  ImVec2 size{0.f, 0.f};
  ImVec2 windowPadding = Theme::DefaultWindowPadding;
  ImVec2 itemSpacing = Theme::DefaultItemSpacing;
  ImVec2 framePadding = Theme::DefaultFramePadding;
  ImU32 bg = Theme::WindowBg;
  ImU32 text = Theme::TextColor;
  ImGuiWindowFlags extraFlags = 0;
};

class RetroWindow {
public:
  RetroWindow(const char *id, const RetroWindowDesc &desc) {
    ImGui::SetNextWindowPos(desc.pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(desc.size, ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {0.f, 0.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, desc.windowPadding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, desc.itemSpacing);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, desc.framePadding);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, desc.bg);
    ImGui::PushStyleColor(ImGuiCol_Text, desc.text);
    ImGui::PushStyleColor(ImGuiCol_Button, Theme::ButtonBg);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::ButtonHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::ButtonActive);

    static constexpr ImGuiWindowFlags kBaseFlags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin(id, nullptr, kBaseFlags | desc.extraFlags);

    m_drawList = ImGui::GetWindowDrawList();
    m_min = ImGui::GetWindowPos();
    m_max = {m_min.x + ImGui::GetWindowWidth(),
             m_min.y + ImGui::GetWindowHeight()};
  }

  ~RetroWindow() {
    ImGui::End();
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(4);
  }

  RetroWindow(const RetroWindow &) = delete;
  RetroWindow &operator=(const RetroWindow &) = delete;

  ImDrawList *drawList() const { return m_drawList; }
  ImVec2 min() const { return m_min; }
  ImVec2 max() const { return m_max; }

private:
  ImDrawList *m_drawList = nullptr;
  ImVec2 m_min{};
  ImVec2 m_max{};
};

} // namespace UI
