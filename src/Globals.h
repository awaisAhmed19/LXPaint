#pragma once
#include <SDL3/SDL.h>
#include <cmath>
#include <concepts>
#include <stdint.h>

// #define LIGHTGRAY {200, 200, 200, 255} // Light Gray
// #define GRAY {130, 130, 130, 255}      // Gray
// #define DARKGRAY {80, 80, 80, 255}     // Dark Gray
// #define YELLOW {253, 249, 0, 255}      // Yellow
// #define GOLD {255, 203, 0, 255}        // Gold
// #define ORANGE {255, 161, 0, 255}      // Orange
// #define PINK {255, 109, 194, 255}      // Pink
// #define RED {230, 41, 55, 255}         // Red
// #define MAROON {190, 33, 55, 255}      // Maroon
// #define GREEN {0, 228, 48, 255}        // Green
// #define LIME {0, 158, 47, 255}         // Lime
// #define DARKGREEN {0, 117, 44, 255}    // Dark Green
// #define SKYBLUE {102, 191, 255, 255}   // Sky Blue
// #define BLUE {0, 121, 241, 255}        // Blue
// #define DARKBLUE {0, 82, 172, 255}     // Dark Blue
// #define PURPLE {200, 122, 255, 255}    // Purple
// #define VIOLET {135, 60, 190, 255}     // Violet
// #define DARKPURPLE {112, 31, 126, 255} // Dark Purple
// #define BEIGE {211, 176, 131, 255}     // Beige
// #define BROWN {127, 106, 79, 255}      // Brown
// #define DARKBROWN {76, 63, 47, 255}    // Dark Brown
//
// #define WHITE {255, 255, 255, 255} // White
// #define BLACK {0, 0, 0, 255}       // Black
// #define BLANK {0, 0, 0, 0}         // Blank (Transparent)
// #define MAGENTA {255, 0, 255, 255} // Magenta

extern int WindowWidth;
extern int WindowHeight;

enum class LineAlgo { BRESENHAM, DDA };
extern LineAlgo g_CurrentAlgo; // Global so shortcuts and tools both see it

struct vec2 {
  float x = 0;
  float y = 0;

  // Standard constructor
  vec2(float _x = 0, float _y = 0) : x(_x), y(_y) {}

  // Binary Operators
  vec2 operator+(const vec2 &other) const { return {x + other.x, y + other.y}; }
  vec2 operator+(const float scalar) const { return {x + scalar, y + scalar}; }
  vec2 operator-(const float scalar) const { return {x - scalar, y - scalar}; }

  // Equality
  bool operator==(const vec2 &other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const vec2 &other) const {
    return x != other.x || y != other.y;
  }
};

typedef struct {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
} vec3;

// typedef struct {
//   float x = 0.0f;
//   float y = 0.0f;
//   float w = 0.0f;
//   float h = 0.0f;
// } Rect;

typedef struct {
  void *data = nullptr;
  int width = 0;
  int height = 0;
  int format = 0;
} Image;

namespace Config {
inline constexpr bool ENABLE_DEBUG_LOGS = true;
inline constexpr bool ENABLE_PERFORMANCE_LOGS = true;
} // namespace Config
namespace COLORS {
// SDL3 uses ARGB8888 or ABGR8888 depending on platform,
// but 0xAARRGGBB is standard for software surfaces.
const uint32_t RED = 0xFFFF0000;
const uint32_t GREEN = 0xFF00FF00;
const uint32_t BLUE = 0xFF0000FF;
const uint32_t BLACK = 0xFF000000;
const uint32_t WHITE = 0xFFFFFFFF;
const uint32_t CLEAR = 0x00000000;
} // namespace COLORS
inline bool lockSurface(SDL_Surface *surface) {
  if (SDL_MUSTLOCK(surface)) {
    if (SDL_LockSurface(surface) < 0)
      return false;
  }
  return true;
}

inline void unlockSurface(SDL_Surface *surface) {
  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
  }
}
