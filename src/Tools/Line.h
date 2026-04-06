#pragma once
#include <math.h>

#include <iostream>
#include <ostream>
#include <vector>

#include "LineCommand.h"
#include "raylib.h"

class LineTool {
  Vector2 startpos, endpos;
  Image preSnapshot;
  bool drawing = false;
  bool snapshotTaken = false;

public:
  Color color = BLACK;
  int brushSize = 2;
  void handlemouseDown(Canvas &canvas) {
    drawing = true;
    snapshotTaken = false;
    startpos = GetMousePosition();
    // snapshot BEFORE anything is drawn
    preSnapshot = canvas.captureSnapshot();
    snapshotTaken = true;
    std::cout << "Line drawing start" << std::endl;
  }
  void handleDraw(Canvas &canvas) {
    endpos = GetMousePosition();
    if ((int)startpos.x != (int)endpos.x || (int)startpos.y != (int)endpos.y) {
      canvas.clearBuffer(); // ← clear old preview first
      canvas.beginDrawBuffer();
      bresenham(startpos, endpos, color, brushSize);
      canvas.endDraw();
    }
  }

  LineCommand *handlemouseUp(Canvas &canvas) {
    drawing = false;
    if (snapshotTaken) {
      canvas.commitBuffer();
      return new LineCommand(preSnapshot, startpos, endpos, color, brushSize);
    }
  }
  LineCommand *draw(Canvas &canvas) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
      handlemouseDown(canvas);
    if (drawing && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
      handleDraw(canvas);
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && drawing)
      return handlemouseUp(canvas);
    else {
      return nullptr;
    }
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
      // std::cout << "bresenham called" << std::endl;
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
