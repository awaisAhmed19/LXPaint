#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "../canvas.h"
#include "../command.h"
#include "raylib.h"

class LineCommand: public Command {
    Image snapshot;  // canvas state BEFORE the stroke
    Vector2 startpos,endpos;
    Color color;
    int brushSize;

   public:
    const std::string toolName = "Line";
    LineCommand(Image snapshot, Vector2 startpos,Vector2 endpos, Color color, int brushSize)
        : snapshot(snapshot), startpos(startpos),endpos(endpos), color(color), brushSize(brushSize) {}

    ~LineCommand() { UnloadImage(snapshot); }

    void execute(Canvas& c) override {
        // stroke already drawn in realtime — nothing to do here
        std::cout << "[LX_INFO]: Command Committed:" << toolName << std::endl;
    }

    void undo(Canvas& c) override { c.restoreSnapshot(snapshot); }
};
