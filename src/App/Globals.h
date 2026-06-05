#pragma once

#include <SDL3/SDL.h>

#include <cstdint>

typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
/*
========================================
Core Math Types
========================================
*/

struct vec2 {

  float x = 0.0f;
  float y = 0.0f;

  constexpr vec2() = default;

  constexpr vec2(float _x, float _y) : x(_x), y(_y) {}

  /*
    Vector arithmetic
  */

  constexpr vec2 operator+(const vec2 &other) const {

    return {x + other.x, y + other.y};
  }

  constexpr vec2 operator-(const vec2 &other) const {

    return {x - other.x, y - other.y};
  }

  constexpr vec2 operator*(float scalar) const {

    return {x * scalar, y * scalar};
  }

  constexpr vec2 operator/(float scalar) const {

    return {x / scalar, y / scalar};
  }

  constexpr vec2 &operator+=(const vec2 &other) {

    x += other.x;
    y += other.y;

    return *this;
  }

  constexpr vec2 &operator-=(const vec2 &other) {

    x -= other.x;
    y -= other.y;

    return *this;
  }

  constexpr bool operator==(const vec2 &other) const {

    return x == other.x && y == other.y;
  }

  constexpr bool operator!=(const vec2 &other) const {

    return !(*this == other);
  }
};

struct dim {
  uint16_t w;
  uint16_t h;
};

struct coords {
  int x;
  int y;
  constexpr coords() = default;

  constexpr coords(int _x, int _y) : x(_x), y(_y) {}

  constexpr coords operator+(const coords &other) const {
    return {x + other.x, y + other.y};
  }

  constexpr coords operator-(const coords &other) const {
    return {x - other.x, y - other.y};
  }

  constexpr coords operator*(float scalar) const {

    return {(int)(x * scalar), (int)(y * scalar)};
  }

  constexpr coords operator/(float scalar) const {

    return {(int)(x / scalar), (int)(y / scalar)};
  }

  constexpr coords &operator+=(const coords &other) {

    x += other.x;
    y += other.y;

    return *this;
  }

  constexpr coords &operator-=(const coords &other) {

    x -= other.x;
    y -= other.y;

    return *this;
  }

  constexpr bool operator==(const coords &other) const {

    return x == other.x && y == other.y;
  }

  constexpr bool operator!=(const coords &other) const {

    return !(*this == other);
  }
};
struct vec3 {

  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

/*
========================================
Raster Algorithms
========================================
*/

enum class LineAlgo { BRESENHAM, DDA };

extern LineAlgo g_CurrentAlgo;

/*
========================================
Engine Config
========================================
*/

namespace Config {

inline bool ENABLE_DEBUG_LOGS = true;

inline bool ENABLE_ASSERTS = true;

inline bool ENABLE_PERFORMANCE_LOGS = true;

} // namespace Config

/*
========================================
Engine Colors
========================================
*/

namespace COLORS {

constexpr uint32_t RED = 0xFFFF0000;

constexpr uint32_t GREEN = 0xFF00FF00;

constexpr uint32_t BLUE = 0xFF0000FF;

constexpr uint32_t BLACK = 0xFF000000;

constexpr uint32_t WHITE = 0xFFFFFFFF;

constexpr uint32_t CLEAR = 0x00000000;

} // namespace COLORS

/*
========================================
SDL Surface Utilities
========================================
*/

inline bool lockSurface(SDL_Surface *surface) {

  if (!surface)
    return false;

  if (SDL_MUSTLOCK(surface)) {

    if (!SDL_LockSurface(surface))
      return false;
  }

  return true;
}

inline void unlockSurface(SDL_Surface *surface) {

  if (!surface)
    return;

  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }
}
