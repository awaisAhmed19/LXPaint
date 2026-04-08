#include "Pencil.h"
#include "../commands/DrawCommand.h"
void Pencil::onMouseDown(vec2 pos, Canvas& canvas) {
    drawing = true;
    startpos = pos;

    // CRITICAL: Capture the canvas BEFORE we draw anything
    if (currentSnapshot) SDL_DestroySurface(currentSnapshot);
    currentSnapshot = SDL_DuplicateSurface(canvas.drawingSurface);

    PutPixel(canvas.drawingSurface, startpos, color);
}

void Pencil::onMouseMove(vec2 pos, Canvas& canvas) {
    if (!drawing) return;

    vec2 currentPos = pos;
    // Connect last position to current position
    bresenham(startpos, currentPos, canvas, color);

    startpos = currentPos; // Update for the next frame
}

Command* Pencil::onMouseUp(vec2 pos, Canvas& canvas) {
    drawing = false;

    // Pass the captured "before" and the current "after"
    // DrawCommand will make its own permanent copies
    Command* cmd = new DrawCommand(currentSnapshot, canvas.drawingSurface);

    // Clean up the temporary snapshot used during this stroke
    SDL_DestroySurface(currentSnapshot);
    currentSnapshot = nullptr;

    return cmd;
}

void Pencil::bresenham(vec2 start, vec2 end, Canvas& canvas, uint32_t color) {
    int dx = abs(end.x - start.x);
    int dy = abs(end.y - start.y);
    int sx = (start.x < end.x) ? 1 : -1;
    int sy = (start.y < end.y) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        PutPixel(canvas.drawingSurface, start, color);

        if (start.x == end.x && start.y == end.y) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            start.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            start.y += sy;
        }
    }
}
