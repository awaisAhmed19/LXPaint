#pragma once
#include <SDL3/SDL.h>
#include <concepts>
#include <stdint.h>
extern int WindowWidth;
extern int WindowHeight;

enum class LineAlgo { BRESENHAM, DDA };
extern LineAlgo g_CurrentAlgo; // Global so shortcuts and tools both see it

template <typename T>
  requires std::floating_point<T> || std::integral<T>
struct vec2 {
  T x = 0;
  T y = 0;

  // Standard constructor
  vec2(T _x = 0, T _y = 0) : x(_x), y(_y) {}

  // Binary Operators
  vec2 operator+(const vec2 &other) const { return {x + other.x, y + other.y}; }
  vec2 operator+(const T scalar) const { return {x + scalar, y + scalar}; }
  vec2 operator-(const T scalar) const { return {x - scalar, y - scalar}; }

  // Equality
  bool operator==(const vec2 &other) const {
    return x == other.x && y == other.y;
  }
};
using vec2i = vec2<int>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;

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

// Using 'inline' to allow the body to stay in the header file
inline void PutPixel(SDL_Surface *surface, vec2i pos, uint32_t color) {
  if (pos.x < 0 || pos.x >= surface->w || pos.y < 0 || pos.y >= surface->h)
    return;

  uint32_t *pixels = (uint32_t *)surface->pixels;
  // We use pitch/4 because pitch is in bytes, and we are using 4-byte uint32_t
  pixels[(pos.y * (surface->pitch / 4)) + pos.x] = color;
}
