
#pragma once

#include "../App/Globals.h"

#include <chrono>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

enum class LogLevel { INFO, WARNING, ERR, DEBUG };

class Logger {
public:
  static inline std::vector<std::string> history;
  static inline std::mutex logMutex;
  static inline bool enabled = true;

  static inline std::ofstream logFile;
  static inline bool initialized = false;

  static void init(const std::string &filename = "runtime.log") {
    std::lock_guard<std::mutex> lock(logMutex);

    if (initialized)
      return;

    logFile.open(filename, std::ios::out | std::ios::app);

    initialized = true;
  }

  static const char *levelToString(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:
      return "INFO";
    case LogLevel::WARNING:
      return "WARN";
    case LogLevel::ERR:
      return "ERROR";
    case LogLevel::DEBUG:
      return "DEBUG";
    }

    return "UNKNOWN";
  }

  static const char *levelToColor(LogLevel level) {
    switch (level) {
    case LogLevel::INFO:
      return "\033[32m";
    case LogLevel::WARNING:
      return "\033[33m";
    case LogLevel::ERR:
      return "\033[31m";
    case LogLevel::DEBUG:
      return "\033[36m";
    }

    return "\033[0m";
  }
  static void log(LogLevel level, std::string_view message) {
    if (!enabled)
      return;

    if (!initialized)
      init();

    auto now = std::chrono::system_clock::now();

    std::string fullMessage =
        std::format("[{:%T}] [{}] {}", now, levelToString(level), message);

    std::lock_guard<std::mutex> lock(logMutex);

    history.push_back(fullMessage);

    if (history.size() > 500) {
      history.erase(history.begin());
    }

    std::cout << levelToColor(level) << fullMessage << "\033[0m" << std::endl;

    if (logFile.is_open()) {
      logFile << fullMessage << std::endl;
      logFile.flush();
    }
  }

  static void debug(std::string_view msg) {
    if constexpr (Config::ENABLE_DEBUG_LOGS) {
      log(LogLevel::DEBUG, msg);
    }
  }
};
