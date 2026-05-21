#pragma once

#include "App/Globals.h"
#include "Logger.h"

#include <SDL3/SDL.h>
#include <cstdlib>
#include <format>

namespace Assert {

inline void fail(const char *condition, const char *message, const char *file,
                 int line) {

  Logger::log(LogLevel::ERR, std::format("ASSERT FAILED\n"
                                         "Condition : {}\n"
                                         "Message   : {}\n"
                                         "File      : {}\n"
                                         "Line      : {}",
                                         condition, message, file, line));

  SDL_TriggerBreakpoint();

  std::abort();
}

} // namespace Assert

#define LX_ASSERT(condition, message)                                          \
  do {                                                                         \
    if (Config::ENABLE_ASSERTS && !(condition)) {                              \
      Assert::fail(#condition, message, __FILE__, __LINE__);                   \
    }                                                                          \
  } while (0)
