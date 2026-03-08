
#include <iostream>

#include "raylib.h"

namespace app {
const int SWIDTH = 800;
const int SHEIGHT = 800;
const char* title = "LX paint";
const Color background = {130, 130, 130, 255};
bool drawing = false;
Vector2 startPos, endPos = {0, 0};
}  // namespace app

void bresenham(Vector2 start, Vector2 end, Color activeColor) {
    int dx = abs((int)end.x - (int)start.x);
    int dy = abs((int)end.y - (int)start.y);
    int sx = (start.x < end.x) ? 1 : -1;
    int sy = (start.y < end.y) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        std::cout << "{" << start.x << "," << start.y << "}" << std::endl;

        DrawRectangle(start.x, start.y, 1, 1, {0, 0, 0, 255});
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
    // time comp=O(x2-x1)
    // auxiliry space : O(1)
}

void MouseDown() {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        app::startPos = GetMousePosition();
        app::drawing = true;
    }
}

void MouseMove() {
    if (app::drawing) {
        app::endPos = GetMousePosition();
        bresenham(app::startPos, app::endPos, {0, 0, 0, 255});
        app::startPos = app::endPos;
    }
}
void MouseReleased() {
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && app::drawing) {
        app::endPos = GetMousePosition();
        bresenham(app::startPos, app::endPos, {0, 0, 0, 255});
        app::drawing = false;
    }
}
int main(void) {
    InitWindow(app::SWIDTH, app::SHEIGHT, app::title);
    SetTargetFPS(60);

    int monitor = GetCurrentMonitor();
    std::cout << "Monitor: " << GetMonitorName(monitor) << std::endl;

    RenderTexture2D target = LoadRenderTexture(app::SWIDTH - 100, app::SHEIGHT - 100);

    // Clear canvas to white once at start
    BeginTextureMode(target);
    ClearBackground(RAYWHITE);
    EndTextureMode();  // ← not EndVrStereoMode!

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        BeginTextureMode(target);
        MouseDown();
        MouseMove();
        MouseReleased();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(app::background);
        DrawTexturePro(
            target.texture,
            (Rectangle){0, 0, (float)target.texture.width, (float)-target.texture.height},
            (Rectangle){0, 0, (float)app::SWIDTH - 100, (float)app::SHEIGHT - 100}, (Vector2){0, 0},
            0, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}
