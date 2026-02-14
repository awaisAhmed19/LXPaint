// Canvas is a pixel buffer which will be the main component of the engine that will make this
// Author: Awais Ahmed
// Date: 11/02/2026
// Texture is an img stored in a gpu memory
// cannot be directly edit on the pixel level easily
// pipeline: creating our own pixel buffer of uint32_t and use textures and renderer to display it
// onto the screen
//
#pragma once
#include <stdlib.h>

#include <cinttypes>
#include <cstddef>
#include <iterator>
#include <vector>
struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct Canvas {
    size_t width;
    size_t height;
    std::vector<uint32_t> buffer;
};

Canvas createCanvas(size_t w, size_t h);
void fillWhite(Canvas& c);
