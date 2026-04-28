#pragma once
#include "../App.h"
#include "../Core/Canvas.h"
#include "../Core/Logger.h"
#include "../Core/Profiler.h"
#include "../Globals.h"
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
  // 1. Setup Window
  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.6f, 20),
                          ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(550, 400), ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Scientific Comparison Benchmark")) {
    if (ImGui::Button("Clear History")) {
      Profiler::comparisonStorage.clear();
    }

    ImGui::SameLine();
    ImGui::Text(" | Legend: ");
    ImGui::SameLine();

    // --- Legend Labels ---
    // We explicitly mention these here for your survey documentation
    ImGui::ColorButton("##bresenham_color", ImVec4(1, 0.5f, 0, 1),
                       ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
    ImGui::SameLine();
    ImGui::Text("Bresenham (Orange)");

    ImGui::SameLine();

    ImGui::ColorButton("##dda_color", ImVec4(0, 0.5f, 1, 1),
                       ImGuiColorEditFlags_NoTooltip, ImVec2(15, 15));
    ImGui::SameLine();
    ImGui::Text("DDA (Blue)");

    ImGui::Separator();

    // --- Data Pre-calculation ---
    float maxUs = 10.0f;
    float maxDist = 1.0f;
    for (auto &run : Profiler::comparisonStorage) {
      maxDist = std::max(maxDist, run.totalDistance);
      for (float p : run.dataPoints)
        maxUs = std::max(maxUs, p);
    }

    // --- Drawing Canvas Geometry ---
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 startPos = ImGui::GetCursorScreenPos();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // Margin logic to ensure numbers fit
    float marginLeft = 50.0f;   // Space for "100us"
    float marginBottom = 40.0f; // Space for "500px"

    // 'cp' is the Top-Left of the actual black graph box
    ImVec2 cp = ImVec2(startPos.x + marginLeft, startPos.y + 10);
    // 'sz' is the width/height of the actual black graph box
    ImVec2 sz = ImVec2(contentSize.x - marginLeft - 20,
                       contentSize.y - marginBottom - 10);

    // Background Grid
    drawList->AddRectFilled(cp, ImVec2(cp.x + sz.x, cp.y + sz.y),
                            IM_COL32(10, 10, 10, 255));
    drawList->AddRect(cp, ImVec2(cp.x + sz.x, cp.y + sz.y),
                      IM_COL32(100, 100, 100, 150));

    // --- Axis Numbering & Grid ---
    int divisions = 5;
    for (int i = 0; i <= divisions; i++) {
      float t = (float)i / (float)divisions;

      // Y-Axis (Latency)
      float yPos = cp.y + sz.y - (t * sz.y);
      drawList->AddLine(ImVec2(cp.x, yPos), ImVec2(cp.x + sz.x, yPos),
                        IM_COL32(40, 40, 40, 255));
      std::string yLabel = std::to_string((int)(t * maxUs)) + "us";
      drawList->AddText(ImVec2(startPos.x + 5, yPos - 7),
                        IM_COL32(200, 200, 200, 255), yLabel.c_str());

      // X-Axis (Distance)
      float xPos = cp.x + (t * sz.x);
      drawList->AddLine(ImVec2(xPos, cp.y), ImVec2(xPos, cp.y + sz.y),
                        IM_COL32(40, 40, 40, 255));
      std::string xLabel = std::to_string((int)(t * maxDist)) + "px";
      drawList->AddText(ImVec2(xPos - 15, cp.y + sz.y + 8),
                        IM_COL32(200, 200, 200, 255), xLabel.c_str());
    }

    // --- Render the Run Lines ---
    for (auto &run : Profiler::comparisonStorage) {
      if (run.dataPoints.size() < 2)
        continue;

      for (size_t i = 0; i < run.dataPoints.size() - 1; i++) {
        // Map the distance and latency to our offset canvas (cp and sz)
        float x1_norm = run.distances[i] / maxDist;
        float x2_norm = run.distances[i + 1] / maxDist;
        float y1_norm = run.dataPoints[i] / maxUs;
        float y2_norm = run.dataPoints[i + 1] / maxUs;

        ImVec2 p1 =
            ImVec2(cp.x + x1_norm * sz.x, cp.y + sz.y - (y1_norm * sz.y));
        ImVec2 p2 =
            ImVec2(cp.x + x2_norm * sz.x, cp.y + sz.y - (y2_norm * sz.y));

        drawList->AddLine(p1, p2, ImGui::ColorConvertFloat4ToU32(run.color),
                          2.5f);
      }
    }

    // Reserve the space in the window
    ImGui::Dummy(contentSize);
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
