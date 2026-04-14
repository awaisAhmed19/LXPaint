#pragma once
#include "../App.h"
#include "../Globals.h"
#include "../core/Canvas.h"
#include "../core/Logger.h"
#include "../core/Profiler.h"
#include "imgui.h"
#include <algorithm>

void RenderAuditLogs(float width, float height) {
  ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - height));
  ImGui::SetNextWindowSize(ImVec2(width, height));
  if (ImGui::Begin("Audit Logs", nullptr,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    if (ImGui::Button("Clear")) {
      std::lock_guard<std::mutex> lock(Logger::logMutex);
      Logger::history.clear();
    }
    ImGui::BeginChild("LogScroll");
    std::lock_guard<std::mutex> lock(Logger::logMutex);
    for (const auto &line : Logger::history)
      ImGui::TextUnformatted(line.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
  }
  ImGui::End();
}

// --- Helper: Render the Global FPS Monitor ---
void RenderFPSMonitor(float xPos, float width, float height) {
  ImGui::SetNextWindowPos(ImVec2(xPos, ImGui::GetIO().DisplaySize.y - height));
  ImGui::SetNextWindowSize(ImVec2(width, height));
  if (ImGui::Begin("FPS Monitor", nullptr,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    float latestMs = App::frameTimes[(App::frameOffset + 99) % 100];
    float currentFPS = (latestMs > 0) ? (1000.0f / latestMs) : 0;

    ImGui::Text("Global Health");
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Latency: %.2f ms", latestMs);
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %.1f", currentFPS);
    ImGui::Separator();
    ImGui::PlotHistogram("##FPSHist", App::frameTimes, 100, App::frameOffset,
                         nullptr, 0.0f, 100.0f, ImVec2(-1, 60));
  }
  ImGui::End();
}

// --- Helper: Render the Live Algo Profiler ---
void RenderLiveProfiler(float xPos, float width, float height) {
  ImGui::SetNextWindowPos(ImVec2(xPos, ImGui::GetIO().DisplaySize.y - height));
  ImGui::SetNextWindowSize(ImVec2(width, height));
  if (ImGui::Begin("Algorithm Profiler", nullptr,
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
    if (Profiler::algoData.empty())
      ImGui::Text("No data recorded.");
    for (auto &[name, stats] : Profiler::algoData) {
      float maxUs = 0;
      for (int i = 0; i < 100; i++)
        maxUs = std::max(maxUs, stats.history[i]);
      ImGui::Text("%s: %.0f us", name.c_str(),
                  stats.history[(stats.offset + 99) % 100]);
      ImGui::PlotLines("##", stats.history, 100, stats.offset, nullptr, 0.0f,
                       maxUs + 5.0f, ImVec2(-1, 40));
    }
  }
  ImGui::End();
}

void RenderComparisonBenchmark() {
  // 1. Setup Window (Positioned at the top right area by default)
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.6f, 20),
                          ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(500, 320), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Scientific Comparison Benchmark")) {
    if (ImGui::Button("Clear Benchmark History")) {
      Profiler::comparisonStorage.clear();
    }

    ImGui::SameLine();
    ImGui::Text(" |  Legend: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Bresenham");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "DDA");

    ImGui::Separator();

    // --- Data Pre-calculation ---
    float maxUs = 10.0f;  // Floor at 10us for visibility
    float maxDist = 1.0f; // Avoid div by zero

    for (auto &run : Profiler::comparisonStorage) {
      maxDist = std::max(maxDist, run.totalDistance);
      for (float p : run.dataPoints)
        maxUs = std::max(maxUs, p);
    }

    // --- Drawing Canvas Setup ---
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 cp = ImGui::GetCursorScreenPos();
    ImVec2 sz = ImGui::GetContentRegionAvail();
    sz.y -= 20; // Leave room for footer text

    // Background Grid
    drawList->AddRectFilled(cp, ImVec2(cp.x + sz.x, cp.y + sz.y),
                            IM_COL32(20, 20, 20, 255));
    drawList->AddRect(cp, ImVec2(cp.x + sz.x, cp.y + sz.y),
                      IM_COL32(100, 100, 100, 100));

    // --- Render the Runs ---
    for (auto &run : Profiler::comparisonStorage) {
      if (run.dataPoints.size() < 3)
        continue;

      // Simple 3-point moving average to eliminate OS background jitter
      auto getSmooth = [&](int i) {
        if (i <= 0 || i >= (int)run.dataPoints.size() - 1)
          return run.dataPoints[i];
        return (run.dataPoints[i - 1] + run.dataPoints[i] +
                run.dataPoints[i + 1]) /
               3.0f;
      };

      for (size_t i = 0; i < run.dataPoints.size() - 1; i++) {
        // SCIENTIFIC X-AXIS: Map point index to actual physical distance
        // traveled This allows a fast 100px line to overlap a slow 100px line
        // perfectly.
        float x1_norm = (run.distances[i] / maxDist);
        float x2_norm = (run.distances[i + 1] / maxDist);

        ImVec2 p1 = ImVec2(cp.x + x1_norm * sz.x,
                           cp.y + sz.y - (getSmooth(i) / maxUs) * sz.y);
        ImVec2 p2 = ImVec2(cp.x + x2_norm * sz.x,
                           cp.y + sz.y - (getSmooth(i + 1) / maxUs) * sz.y);

        drawList->AddLine(p1, p2, ImGui::ColorConvertFloat4ToU32(run.color),
                          2.0f);
      }
    }

    // --- Axis Labels & Overlay ---
    drawList->AddText(ImVec2(cp.x + 10, cp.y + 10),
                      IM_COL32(200, 200, 200, 180), "Y: Latency (us)");
    drawList->AddText(ImVec2(cp.x + sz.x - 120, cp.y + sz.y - 25),
                      IM_COL32(200, 200, 200, 180), "X: Distance (px)");

    // Finalize the reserved space for the custom drawList
    ImGui::Dummy(sz);

    if (Profiler::comparisonStorage.empty()) {
      ImGui::SetCursorPos(ImVec2(sz.x * 0.35f, sz.y * 0.5f));
      ImGui::TextDisabled("No strokes recorded for comparison.");
    }
  }
  ImGui::End();
}
// --- MAIN ENTRY POINT ---
inline void DrawLogConsole(const Canvas &canvas, int WindowWidth,
                           int WindowHeight) {
  float h = WindowHeight * 0.3f;
  float wLog = WindowWidth * 0.4f;
  float wFPS = WindowWidth * 0.2f;
  float wAlgo = WindowWidth * 0.4f;

  RenderAuditLogs(wLog, h);
  RenderFPSMonitor(wLog, wFPS, h);
  RenderLiveProfiler(wLog + wFPS, wAlgo, h);
  RenderComparisonBenchmark();
}
