#pragma once
#include <chrono>
#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "../../external/imgui/imgui.h"
#include "../Globals.h"
struct AlgoRun {
    std::string name;
    std::vector<float> dataPoints;
    std::vector<float> distances;
    float totalDistance = 0.0f;
    ImVec4 color;
};

struct AlgoStats {
    float history[100] = {0};
    int offset = 0;
};

// Data container for a single "Step" in a race
struct RaceResult {
    std::string name;
    float microseconds;
    ImVec4 color;
};
struct FillResult {
    std::string name;
    float microseconds;
    int pixelsFilled;
    ImVec4 color;
};
class Profiler {
    std::string funcName;
    std::chrono::high_resolution_clock::time_point start;

   public:
    // We use a map of vectors to allow N-algorithm comparison dynamically
    static inline std::map<std::string, std::vector<float>> raceSessions;
    static inline std::vector<float> currentDistances;
    static inline float sessionDistance = 0.0f;
    static inline vec2<float> lastPos = {-1.0f, -1.0f};

    static inline std::map<std::string, std::vector<std::pair<int, float>>> fillSessions;
    static inline std::vector<AlgoRun> comparisonStorage;
    static inline std::map<std::string, struct AlgoStats> algoData;

    Profiler(std::string name) : funcName(name) {
        start = std::chrono::high_resolution_clock::now();
    }
    // Inside the Profiler class:

    /**
     * @brief Records performance of a Flood Fill operation.
     * @param pixelsFilled The total count of pixels modified.
     * @param microseconds Execution time.
     */
    static void recordFill(std::string name, int pixelsFilled, float microseconds,
                           ImVec4 color = {1, 1, 1, 1}) {
        AlgoRun run;
        run.name = name + " (Fill)";
        run.color = color;

        // We reuse dataPoints for time and distances for pixel count
        // to maintain compatibility with your existing UI plotter
        run.dataPoints.push_back(microseconds);
        run.distances.push_back((float)pixelsFilled);
        run.totalDistance = (float)pixelsFilled;

        comparisonStorage.push_back(run);
    }
    // --- The General Purpose Race Recorder ---
    // Pass a vector of results (e.g., { {name, time, color}, ... })
    static void recordRaceStep(const std::vector<RaceResult>& results, vec2<float> currentPos) {
        if (lastPos.x != -1.0f) {
            float d = std::sqrt(std::pow(currentPos.x - lastPos.x, 2) +
                                std::pow(currentPos.y - lastPos.y, 2));
            sessionDistance += d;
        }

        for (const auto& res : results) {
            raceSessions[res.name].push_back(res.microseconds);
        }

        currentDistances.push_back(sessionDistance);
        lastPos = currentPos;
    }

    // --- Commit all active sessions to permanent storage ---
    static void commitRace(const std::map<std::string, ImVec4>& colors) {
        for (auto& [name, timings] : raceSessions) {
            AlgoRun run;
            run.name = name;
            run.dataPoints = timings;
            run.distances = currentDistances;
            run.totalDistance = sessionDistance;
            run.color = colors.count(name) ? colors.at(name) : ImVec4(1, 1, 1, 1);

            comparisonStorage.push_back(run);
        }

        raceSessions.clear();
        currentDistances.clear();
        sessionDistance = 0.0f;
        lastPos = {-1.0f, -1.0f};
    }

    ~Profiler() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        auto& stats = algoData[funcName];
        stats.history[stats.offset] = (float)duration;
        stats.offset = (stats.offset + 1) % 100;
    }
};

#define PROFILE_FUNCTION() Profiler p(__FUNCTION__)
