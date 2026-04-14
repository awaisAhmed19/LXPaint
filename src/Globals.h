#pragma once
#include <SDL3/SDL.h>
#include <stdint.h>

struct vec2 {
    int x = 0;
    int y = 0;

    vec2 operator+(const vec2& other) const { return {x + other.x, y + other.y}; }
    vec2 operator+(const int other) const { return {x + other, y + other}; }
    vec2 operator-(const int other) const { return {x - other, y - other}; }
    bool operator==(const vec2& other) const { return x == other.x && y == other.y; }
};

namespace Config {
    inline constexpr bool ENABLE_DEBUG_LOGS = true;
    inline constexpr bool ENABLE_PERFORMANCE_LOGS = true;
}
namespace COLORS {
    // SDL3 uses ARGB8888 or ABGR8888 depending on platform,
    // but 0xAARRGGBB is standard for software surfaces.
    const uint32_t RED   = 0xFFFF0000;
    const uint32_t GREEN = 0xFF00FF00;
    const uint32_t BLUE  = 0xFF0000FF;
    const uint32_t BLACK = 0xFF000000;
    const uint32_t WHITE = 0xFFFFFFFF;
    const uint32_t CLEAR = 0x00000000;
}

// Using 'inline' to allow the body to stay in the header file
inline void PutPixel(SDL_Surface* surface, vec2 pos, uint32_t color) {
    if (pos.x < 0 || pos.x >= surface->w || pos.y < 0 || pos.y >= surface->h) return;

    uint32_t* pixels = (uint32_t*)surface->pixels;
    // We use pitch/4 because pitch is in bytes, and we are using 4-byte uint32_t
    pixels[(pos.y * (surface->pitch / 4)) + pos.x] = color;
}
