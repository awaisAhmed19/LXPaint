
#include "./Tools/Line.h"
#include "./Tools/Pencil.h"
#include "canvas.h"
#include "raylib.h"

int main() {
    const int W = 800, H = 600;
    InitWindow(W, H, "LXPaint");
    SetTargetFPS(60);

    Canvas canvas;
    canvas.init(W, H);

    PencilTool pencil;
    LineTool line;

    while (!WindowShouldClose()) {
        // --- update ---
        Command* cmd = line.handleInput(canvas);
        if (cmd) canvas.applyCommand(cmd);

        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)) canvas.undo();

        // --- draw ---
        BeginDrawing();
        ClearBackground(RAYWHITE);
        canvas.render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
