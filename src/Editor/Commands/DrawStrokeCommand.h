#pragma once
#include "../../App/Globals.h"
#include "../../Document/Canvas.h"
#include "../../Rendering/Rasterizer.h"
#include "../Commands/Command.h"
#include <SDL3/SDL.h>
#include <memory>
#include <vector>

class DrawStrokeCommand : public Command {
private:
  struct Segment {
    vec2 start;
    vec2 end;
    uint32_t color;
    int brushSize;
  };

  std::vector<Segment> m_segments;

public:
  void addSegment(vec2 start, vec2 end, uint32_t color, int brushSize) {
    m_segments.push_back({start, end, color, brushSize});
  }

  bool isEmpty() const { return m_segments.empty(); }

  size_t getSegmentCount() const { // ✅ Fixed method
    return m_segments.size();
  }

  void execute(Canvas &canvas) override {
    for (const auto &seg : m_segments) {
      Rasterizer::bresenham(seg.start, seg.end, canvas.getSurface(), seg.color,
                            seg.brushSize, false);
    }
    canvas.markDirty();
  }

  void undo(Canvas &canvas) override {
    // Erase by drawing white (assumes white background)
    for (const auto &seg : m_segments) {
      Rasterizer::bresenham(seg.start, seg.end, canvas.getSurface(),
                            COLORS::WHITE, seg.brushSize, false);
    }
    canvas.markDirty();
  }
};
