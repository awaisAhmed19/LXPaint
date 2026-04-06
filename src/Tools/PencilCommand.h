#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "../canvas.h"
#include "../command.h"
#include "raylib.h"

class PencilCommand : public Command {
  Image snapshot; // canvas state BEFORE the stroke
  std::vector<Vector2> points;
  Color color;
  int brushSize;

public:
  const std::string toolName = "Pencil";
  PencilCommand(Image snapshot, std::vector<Vector2> points, Color color,
                int brushSize)
      : snapshot(snapshot), points(points), color(color), brushSize(brushSize) {
  }

  ~PencilCommand() { UnloadImage(snapshot); }

  void execute(Canvas &c) override {
    // stroke already drawn in realtime — nothing to do here
    std::cout << "[LX_INFO]: Command Committed:" << toolName << std::endl;
  }

  void undo(Canvas &c) override { c.restoreSnapshot(snapshot); }
};
