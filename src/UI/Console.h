#pragma once
#include "../App.h"
#include "../Globals.h"
#include "../core/Canvas.h"
#include "../core/Logger.h"
#include "imgui.h"
#include <algorithm>

inline void DrawLogConsole(const Canvas &canvas, int WindowWidth,
                           int WindowHeight) {
  float panelHeight = WindowHeight * 0.3f;
  float logWidth = WindowWidth * 0.5f;

  // --- LOG WINDOW (Left) ---
  ImGui::SetNextWindowPos(ImVec2(0, WindowHeight - panelHeight));
  ImGui::SetNextWindowSize(ImVec2(logWidth, panelHeight));
  if (ImGui::Begin("Audit Logs", nullptr,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    // Simple Frame Time Number
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Total Frame Latency: %.2f ms",
                       App::frameTimes[(App::frameOffset + 99) % 100]);
    ImGui::Separator();

    ImGui::BeginChild("LogScroll");
    std::lock_guard<std::mutex> lock(Logger::logMutex);
    for (const auto &line : Logger::history)
      ImGui::TextUnformatted(line.c_str());
    ImGui::EndChild();
  }
  ImGui::End();

  // --- ALGORITHM GRAPH WINDOW (Right) ---
  ImGui::SetNextWindowPos(ImVec2(logWidth, WindowHeight - panelHeight));
  ImGui::SetNextWindowSize(ImVec2(WindowWidth - logWidth, panelHeight));
  if (ImGui::Begin("Algorithm Performance", nullptr,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

    for (auto &[name, stats] : Profiler::algoData) {
      // Find max for scaling
      float maxUs = 0;
      for (int i = 0; i < 100; i++)
        if (stats.history[i] > maxUs)
          maxUs = stats.history[i];

      ImGui::Text("%s (Current: %.0f us)", name.c_str(),
                  stats.history[(stats.offset + 99) % 100]);

      // Graph specific to THIS algorithm
      std::string label = "##" + name;
      ImGui::PlotLines(label.c_str(), stats.history, 100, stats.offset, nullptr,
                       0.0f, maxUs + 10.0f, ImVec2(-1, 60));
      ImGui::Spacing();
    }
  }
  ImGui::End();
}
