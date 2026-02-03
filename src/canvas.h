// Canvas is a pixel buffer which will be the main component of the engine that will make this
// applications back bone
#pragma once
#include <iterator>
#include <vector>
struct canvas {
    std::vector<int> data;
    int w, h;
};
