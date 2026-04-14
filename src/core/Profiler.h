#pragma once
#include "./Logger.h"
#include <chrono>
#include <map>
#include <string>
#include <vector>

struct AlgoStats {
  float history[100] = {0};
  int offset = 0;
};

class Profiler {
  std::string funcName;
  std::chrono::high_resolution_clock::time_point start;

public:
  static inline std::map<std::string, AlgoStats> algoData;

  Profiler(std::string name) : funcName(name) {
    start = std::chrono::high_resolution_clock::now();
  }

  ~Profiler() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start)
            .count();

    auto &stats = algoData[funcName];
    stats.history[stats.offset] = (float)duration;
    stats.offset = (stats.offset + 1) % 100;

    // Still log to console for the "Audit Trail"
    // Logger::log(LogLevel::DEBUG, std::format("{} took {}us", funcName,
    // duration));
  }
};

#define PROFILE_FUNCTION() Profiler p(__FUNCTION__)
