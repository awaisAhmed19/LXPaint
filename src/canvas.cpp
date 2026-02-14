
#include <algorithm>
#include <cstdlib>

#include "canvas.h"

Canvas createCanvas(size_t w, size_t h) {
    Canvas canvas;
    canvas.width = w;
    canvas.height = h;
    canvas.buffer.resize(w * h);
    return canvas;
}

void fillWhite(Canvas& c) { std::fill(c.buffer.begin(), c.buffer.end(), 0xFFFFFFFF); }
