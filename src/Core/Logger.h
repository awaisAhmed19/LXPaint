#pragma once
#include "../Globals.h" // To access Config::ENABLE_DEBUG_LOGS
#include <chrono>
#include <format>
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

  static void log(LogLevel level, const std::string_view message) {
    if (!enabled)
      return;

    auto now = std::chrono::system_clock::now();
    std::string levelStr;
    std::string colorCode;

    switch (level) {
    case LogLevel::INFO:
      levelStr = "[INFO]";
      colorCode = "\033[32m";
      break;
    case LogLevel::WARNING:
      levelStr = "[WARN]";
      colorCode = "\033[33m";
      break;
    case LogLevel::ERR:
      levelStr = "[ERROR]";
      colorCode = "\033[31m";
      break;
    case LogLevel::DEBUG:
      levelStr = "[DEBUG]";
      colorCode = "\033[36m";
      break;
    }

    std::string fullMessage =
        std::format("[{:%T}] {} {}", now, levelStr, message);

    {
      std::lock_guard<std::mutex> lock(logMutex);
      history.push_back(fullMessage);
      if (history.size() > 100) {
        history.erase(history.begin());
      }
    }

    std::cout << colorCode << fullMessage << "\033[0m" << std::endl;
  }

  static void debug(std::string_view msg) {
    if constexpr (Config::ENABLE_DEBUG_LOGS) {
      log(LogLevel::DEBUG, msg);
    }
  }
};
