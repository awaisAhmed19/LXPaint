#pragma once
#include <algorithm>

#include "../App.h"
#include "../core/Canvas.h"
#include "../core/Logger.h"
#include "../core/Profiler.h"
#include "imgui.h"

void RenderAuditLogs(float width, float height) {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - height));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    if (ImGui::Begin("Audit Logs", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (ImGui::Button("Clear")) {
            std::lock_guard<std::mutex> lock(Logger::logMutex);
            Logger::history.clear();
        }
        ImGui::BeginChild("LogScroll");
        std::lock_guard<std::mutex> lock(Logger::logMutex);
        for (const auto& line : Logger::history) ImGui::TextUnformatted(line.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }
    ImGui::End();
}

// --- Helper: Render the Global FPS Monitor ---
void RenderFPSMonitor(float xPos, float width, float height) {
    ImGui::SetNextWindowPos(ImVec2(xPos, ImGui::GetIO().DisplaySize.y - height));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    if (ImGui::Begin("FPS Monitor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        float latestMs = App::frameTimes[(App::frameOffset + 99) % 100];
        float currentFPS = (latestMs > 0) ? (1000.0f / latestMs) : 0;

        ImGui::Text("Global Health");
        ImGui::TextColored(ImVec4(0, 1, 1, 1), "Latency: %.2f ms", latestMs);
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "FPS: %.1f", currentFPS);
        ImGui::Separator();
        ImGui::PlotHistogram("##FPSHist", App::frameTimes, 100, App::frameOffset, nullptr, 0.0f,
                             100.0f, ImVec2(-1, 60));
    }
    ImGui::End();
}

// --- Helper: Render the Live Algo Profiler ---
void RenderLiveProfiler(float xPos, float width, float height) {
    ImGui::SetNextWindowPos(ImVec2(xPos, ImGui::GetIO().DisplaySize.y - height));
    ImGui::SetNextWindowSize(ImVec2(width, height));
    if (ImGui::Begin("Algorithm Profiler", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        if (Profiler::algoData.empty()) ImGui::Text("No data recorded.");
        for (auto& [name, stats] : Profiler::algoData) {
            float maxUs = 0;
            for (int i = 0; i < 100; i++) maxUs = std::max(maxUs, stats.history[i]);
            ImGui::Text("%s: %.0f us", name.c_str(), stats.history[(stats.offset + 99) % 100]);
            ImGui::PlotLines("##", stats.history, 100, stats.offset, nullptr, 0.0f, maxUs + 5.0f,
                             ImVec2(-1, 40));
        }
    }
    ImGui::End();
}

// --- Helper: Render the Scientific Comparison Benchmark ---
void RenderComparisonBenchmark() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, 20),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Scientific Comparison Benchmark")) {
        if (ImGui::Button("Clear History")) {
            Profiler::comparisonStorage.clear();
        }

        ImGui::SameLine();
        ImGui::TextUnformatted(" | Metrics: Time (us) vs. Magnitude (px/pts)");
        ImGui::Separator();

        if (Profiler::comparisonStorage.empty()) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No benchmark data recorded.");
            ImGui::End();
            return;
        }

        // --- Data Pre-calculation ---
        float maxUs = 10.0f;
        float maxMagnitude = 1.0f;
        bool isAreaBased = false;

        for (auto& run : Profiler::comparisonStorage) {
            maxMagnitude = std::max(maxMagnitude, run.totalDistance);
            if (run.name.find("Fill") != std::string_view::npos) isAreaBased = true;
            for (float p : run.dataPoints) maxUs = std::max(maxUs, p);
        }

        // --- Drawing Canvas Setup ---
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 startPos = ImGui::GetCursorScreenPos();
        ImVec2 contentSize = ImGui::GetContentRegionAvail();
        float marginLeft = 60.0f;
        float marginBottom = 65.0f;  // Increased for legend/labels
        ImVec2 cp = ImVec2(startPos.x + marginLeft, startPos.y + 10);
        ImVec2 sz = ImVec2(contentSize.x - marginLeft - 20, contentSize.y - marginBottom - 40);

        // Background
        drawList->AddRectFilled(cp, ImVec2(cp.x + sz.x, cp.y + sz.y), IM_COL32(15, 15, 15, 255));
        drawList->AddRect(cp, ImVec2(cp.x + sz.x, cp.y + sz.y), IM_COL32(80, 80, 80, 255));

        // --- Grid & Axis ---
        int divisions = 5;
        for (int i = 0; i <= divisions; i++) {
            float t = (float)i / (float)divisions;
            float yPos = cp.y + sz.y - (t * sz.y);
            drawList->AddLine(ImVec2(cp.x, yPos), ImVec2(cp.x + sz.x, yPos),
                              IM_COL32(45, 45, 45, 255));
            drawList->AddText(ImVec2(startPos.x + 5, yPos - 7), IM_COL32(200, 200, 200, 255),
                              (std::to_string((int)(t * maxUs)) + "us").c_str());

            float xPos = cp.x + (t * sz.x);
            drawList->AddLine(ImVec2(xPos, cp.y), ImVec2(xPos, cp.y + sz.y),
                              IM_COL32(45, 45, 45, 255));
            std::string unit = isAreaBased ? "pts" : "px";
            drawList->AddText(ImVec2(xPos - 15, cp.y + sz.y + 8), IM_COL32(200, 200, 200, 255),
                              (std::to_string((int)(t * maxMagnitude)) + unit).c_str());
        }

        // --- Multi-Algorithm Rendering & Real Average Calculation ---
        for (auto& run : Profiler::comparisonStorage) {
            if (run.dataPoints.empty()) continue;

            // CALCULATE ACTUAL AVERAGE
            float sum = 0;
            for (float val : run.dataPoints) sum += val;
            float avgUs = sum / (float)run.dataPoints.size();

            // Display Legend with Real Average
            ImGui::ColorButton(run.name.c_str(), run.color, ImGuiColorEditFlags_NoTooltip,
                               ImVec2(12, 12));
            ImGui::SameLine();
            ImGui::Text("%s (Avg: %.2f us)", run.name.c_str(), avgUs);
            ImGui::SameLine();

            // Plot Logic
            if (run.dataPoints.size() >= 2) {
                for (size_t i = 0; i < run.dataPoints.size() - 1; i++) {
                    ImVec2 p1 = ImVec2(cp.x + (run.distances[i] / maxMagnitude) * sz.x,
                                       cp.y + sz.y - (run.dataPoints[i] / maxUs) * sz.y);
                    ImVec2 p2 = ImVec2(cp.x + (run.distances[i + 1] / maxMagnitude) * sz.x,
                                       cp.y + sz.y - (run.dataPoints[i + 1] / maxUs) * sz.y);
                    drawList->AddLine(p1, p2, ImGui::ColorConvertFloat4ToU32(run.color), 2.5f);
                }
            } else {
                float x = (run.totalDistance / maxMagnitude) * sz.x;
                float y = (run.dataPoints[0] / maxUs) * sz.y;
                drawList->AddCircleFilled(ImVec2(cp.x + x, cp.y + sz.y - y), 4.0f,
                                          ImGui::ColorConvertFloat4ToU32(run.color));
            }
        }
        ImGui::Dummy(ImVec2(0, 20));
    }
    ImGui::End();
}
// --- MAIN ENTRY POINT ---
inline void DrawLogConsole(const Canvas& canvas, int WindowWidth, int WindowHeight) {
    float h = WindowHeight * 0.3f;
    float wLog = WindowWidth * 0.4f;
    float wFPS = WindowWidth * 0.2f;
    float wAlgo = WindowWidth * 0.4f;

    RenderAuditLogs(wLog, h);
    RenderFPSMonitor(wLog, wFPS, h);
    RenderLiveProfiler(wLog + wFPS, wAlgo, h);
    RenderComparisonBenchmark();
}
