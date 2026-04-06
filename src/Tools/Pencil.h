#pragma once
#include <math.h>

#include <vector>

#include "PencilCommand.h"
#include "raylib.h"

class PencilTool {
  std::vector<Vector2> currentStroke;
  Image preSnapshot;
  bool drawing = false;
  bool snapshotTaken = false;

public:
  Color color = BLACK;
  const int brushSize = 1;

  PencilCommand *handleInput(Canvas &canvas) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      drawing = true;
      snapshotTaken = false;
      currentStroke.clear();
      currentStroke.push_back(GetMousePosition());

      // snapshot BEFORE anything is drawn
      preSnapshot = canvas.captureSnapshot();
      snapshotTaken = true;
    }

    if (drawing && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 pos = GetMousePosition();
      Vector2 last = currentStroke.back();

      if ((int)pos.x != (int)last.x || (int)pos.y != (int)last.y) {
        // draw in realtime directly onto canvas
        canvas.beginDrawMain();
        bresenham(last, pos, color, brushSize);
        canvas.endDraw();

        currentStroke.push_back(pos);
      }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && drawing) {
      drawing = false;
      if (snapshotTaken && currentStroke.size() >= 2)
        return new PencilCommand(preSnapshot, currentStroke, color, brushSize);
    }

    return nullptr;
  }

private:
  void bresenham(Vector2 start, Vector2 end, Color color, int brushSize) {
    int dx = abs((int)end.x - (int)start.x);
    int dy = abs((int)end.y - (int)start.y);
    int sx = (start.x < end.x) ? 1 : -1;
    int sy = (start.y < end.y) ? 1 : -1;
    int err = dx - dy;
    while (true) {
      DrawRectangle((int)start.x, (int)start.y, brushSize, brushSize, color);
      std::cout << "line drawing" << std::endl;
      if ((int)start.x == (int)end.x && (int)start.y == (int)end.y)
        break;
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
};
