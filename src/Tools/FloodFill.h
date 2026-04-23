#pragma once
#include <SDL3/SDL.h>

#include <queue>
#include <stack>

#include "../Globals.h"
#include "../core/Canvas.h"
#include "../core/Profiler.h"
#include "./BaseTool.h"

enum class FloodFillAlgo { SCANLINE };
class FloodFill : public BaseTool {
    bool drawing = false;
    SDL_Surface* currentSnapshot = nullptr;
    vec2<float> seedPoint;

   public:
    uint32_t fillColor = COLORS::RED;
    uint8_t tolerance = 0;  // Color matching threshold (0-255)
    FloodFillAlgo g_CurrentFloodFillAlgo = FloodFillAlgo::SCANLINE;
    void onMouseDown(vec2<float> pos, Canvas& canvas) override;
    void onMouseMove(vec2<float> pos, Canvas& canvas) override;
    Command* onMouseUp(vec2<float> pos, Canvas& canvas) override;

   private:
    struct Span {
        int y, x1, x2, dy;
    };

    int scanlineFill(vec2<int> seed, uint32_t targetColor, uint32_t replacementColor,
                     Canvas& canvas);
    int recursiveFill(vec2<int> seed, uint32_t targetColor, uint32_t replacementColor,
                      Canvas& canvas);

    bool colorMatch(uint32_t c1, uint32_t c2) const;
    uint32_t getPixel(SDL_Surface* surface, int x, int y) const;
    void putPixel(SDL_Surface* surface, int x, int y, uint32_t color);
};
