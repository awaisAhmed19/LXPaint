#include <algorithm>
#include <chrono>
#include <cmath>

#include "../commands/DrawCommand.h"
#include "../core/Logger.h"
#include "../core/Profiler.h"
#include "FloodFill.h"

void FloodFill::onMouseDown(vec2<float> pos, Canvas& canvas) {
    drawing = true;
    seedPoint = pos;

    // Capture state for Undo/Redo and for the "Race" reset
    if (currentSnapshot) SDL_DestroySurface(currentSnapshot);
    currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);

    Logger::log(LogLevel::DEBUG, "FLOOD FILL: STATE CAPTURED");
}
void FloodFill::onMouseMove(vec2<float> pos, Canvas& canvas) {}
Command* FloodFill::onMouseUp(vec2<float> pos, Canvas& canvas) {
    if (!drawing) return nullptr;
    drawing = false;

    vec2<int> seed = {(int)seedPoint.x, (int)seedPoint.y};

    // 1. Validation
    if (seed.x < 0 || seed.x >= canvas.w || seed.y < 0 || seed.y >= canvas.h) return nullptr;
    uint32_t targetColor = getPixel(canvas.drawingSurface, seed.x, seed.y);
    if (colorMatch(targetColor, fillColor)) return nullptr;

    SDL_Surface* benchmarkReset = SDL_DuplicateSurface(currentSnapshot);

    // --- RACE 1: SCANLINE ---
    int countScanline = 0;
    auto s1 = std::chrono::high_resolution_clock::now();
    countScanline = scanlineFill(seed, targetColor, fillColor, canvas);  // Updated to return count
    auto e1 = std::chrono::high_resolution_clock::now();
    float usScanline = std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();

    SDL_BlitSurface(benchmarkReset, NULL, canvas.drawingSurface, NULL);

    // --- RACE 2: RECURSIVE ---
    int countRecursive = 0;
    auto s2 = std::chrono::high_resolution_clock::now();
    countRecursive =
        recursiveFill(seed, targetColor, fillColor, canvas);  // Updated to return count
    auto e2 = std::chrono::high_resolution_clock::now();
    float usRecursive = std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

    // 3. Update Profiler with Magnitude Data
    // Using the generic recordFill we designed to handle Area (pts)
    Profiler::recordFill("Scanline", countScanline, usScanline, {0.2f, 1.0f, 0.2f, 1.0f});
    Profiler::recordFill("Recursive ", countRecursive, usRecursive, {1.0f, 0.2f, 0.2f, 1.0f});

    // 4. Finalize Winner
    if (g_CurrentFloodFillAlgo != FloodFillAlgo::SCANLINE) {
        SDL_BlitSurface(benchmarkReset, NULL, canvas.drawingSurface, NULL);
        scanlineFill(seed, targetColor, fillColor, canvas);
    } else {
        // SDL_BlitSurface(benchmarkReset, NULL, canvas.drawingSurface, NULL);
        // recursiveFill(seed, targetColor, fillColor, canvas);
    }
    SDL_DestroySurface(benchmarkReset);
    Command* cmd = new DrawCommand(currentSnapshot, canvas.drawingSurface);

    if (currentSnapshot) {
        SDL_DestroySurface(currentSnapshot);
        currentSnapshot = nullptr;
    }

    return cmd;
}

// --- SCANLINE FLOOD FILL (Stack-based, cache-coherent) ---
int FloodFill::scanlineFill(vec2<int> seed, uint32_t targetColor, uint32_t replacementColor,
                            Canvas& canvas) {
    if (!SDL_LockSurface(canvas.drawingSurface)) return 0;
    int pixelsFilled = 0;
    std::stack<Span> spans;
    spans.push({seed.y, seed.x, seed.x, 1});       // down
    spans.push({seed.y + 1, seed.x, seed.x, -1});  // up

    uint32_t* pixels = (uint32_t*)canvas.drawingSurface->pixels;
    int pitch = canvas.drawingSurface->pitch >> 2;
    int w = canvas.drawingSurface->w;
    int h = canvas.drawingSurface->h;

    while (!spans.empty()) {
        Span span = spans.top();
        spans.pop();

        int x1 = span.x1;
        int y = span.y + span.dy;

        if (y < 0 || y >= h) continue;

        uint32_t* row = pixels + (y * pitch);

        // Scan left from seed
        while (x1 >= 0 && colorMatch(row[x1], targetColor)) {
            row[x1] = replacementColor;
            x1--;
            pixelsFilled++;
        }
        x1++;

        // Scan right from seed
        int x2 = span.x1 + 1;
        while (x2 < w && colorMatch(row[x2], targetColor)) {
            row[x2] = replacementColor;
            x2++;
        }
        x2--;

        // Push new spans above and below
        for (int dy : {-span.dy, span.dy}) {
            int ny = y + dy;
            if (ny < 0 || ny >= h) continue;

            int sx = x1;
            while (sx <= x2) {
                // Skip already-filled pixels
                while (sx < x2 && !colorMatch(pixels[ny * pitch + sx], targetColor)) sx++;
                if (!colorMatch(pixels[ny * pitch + sx], targetColor)) break;

                int ex = sx;
                while (ex <= x2 && colorMatch(pixels[ny * pitch + ex], targetColor)) ex++;

                spans.push({y, sx, ex - 1, dy});
                sx = ex + 1;
            }
        }
    }

    SDL_UnlockSurface(canvas.drawingSurface);
    return pixelsFilled;
}

// --- NAIVE RECURSIVE FLOOD FILL (Stack overflow risk, poor locality) ---
int FloodFill::recursiveFill(vec2<int> seed, uint32_t targetColor, uint32_t replacementColor,
                             Canvas& canvas) {
    if (!SDL_LockSurface(canvas.drawingSurface)) return 0;
    int pixelsFilled = 0;
    std::stack<vec2<int>> stack;
    stack.push(seed);

    uint32_t* pixels = (uint32_t*)canvas.drawingSurface->pixels;
    int pitch = canvas.drawingSurface->pitch >> 2;
    int w = canvas.drawingSurface->w;
    int h = canvas.drawingSurface->h;

    while (!stack.empty()) {
        vec2<int> p = stack.top();
        stack.pop();

        if (p.x < 0 || p.x >= w || p.y < 0 || p.y >= h) continue;

        uint32_t& pixel = pixels[p.y * pitch + p.x];
        if (!colorMatch(pixel, targetColor)) continue;

        pixel = replacementColor;

        pixelsFilled++;
        // Push 4-connected neighbors
        stack.push({p.x + 1, p.y});
        stack.push({p.x - 1, p.y});
        stack.push({p.x, p.y + 1});
        stack.push({p.x, p.y - 1});
    }

    SDL_UnlockSurface(canvas.drawingSurface);
    return pixelsFilled;
}

// --- COLOR MATCHING WITH TOLERANCE ---
bool FloodFill::colorMatch(uint32_t c1, uint32_t c2) const {
    if (tolerance == 0) return c1 == c2;

    int r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;

    int dr = r1 - r2, dg = g1 - g2, db = b1 - b2;
    int distSq = dr * dr + dg * dg + db * db;

    // Tolerance maps to Euclidean distance threshold
    int thresholdSq = tolerance * tolerance * 3;  // RGB space diagonal
    return distSq <= thresholdSq;
}

uint32_t FloodFill::getPixel(SDL_Surface* surface, int x, int y) const {
    if (!SDL_LockSurface(surface)) return 0;
    uint32_t* pixels = (uint32_t*)surface->pixels;
    uint32_t color = pixels[y * (surface->pitch >> 2) + x];
    SDL_UnlockSurface(surface);
    return color;
}

void FloodFill::putPixel(SDL_Surface* surface, int x, int y, uint32_t color) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
    if (!SDL_LockSurface(surface)) return;
    uint32_t* pixels = (uint32_t*)surface->pixels;
    pixels[y * (surface->pitch >> 2) + x] = color;
    SDL_UnlockSurface(surface);
}
