#include "Eraser.h"

void Eraser::onMouseDown(vec2<float> pos, Canvas &canvas) {
  drawing = true;
  Start = pos;
  resetBounds(pos, brushSize);

  Renderer::bresenham(pos, pos, canvas, color, brushSize, useXOR);
  Logger::log(LogLevel::DEBUG, "ERASER STARTED DRAWING");
}

void Eraser::onMouseMove(vec2<float> pos, Canvas &canvas) {
  if (!drawing)
    return;

  // Abstracted Logic
  updateBounds(pos, brushSize, canvas.w, canvas.h);

  // Benchmarking
  auto s1 = std::chrono::high_resolution_clock::now();
  Renderer::bresenham(Start, pos, canvas, color, brushSize, useXOR);
  auto e1 = std::chrono::high_resolution_clock::now();

  auto s2 = std::chrono::high_resolution_clock::now();
  // Renderer::dda(Start, pos, canvas, color, brushSize, useXOR);
  auto e2 = std::chrono::high_resolution_clock::now();

  float usBres =
      std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();
  float usDDA =
      std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();

  Profiler::recordRaceStep(
      {{"BRESENHAM", usBres, ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
       {"DDA", usDDA, ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}},
      pos);

  Start = pos;
}

Command *Eraser::onMouseUp(vec2<float> pos, Canvas &canvas) {
  drawing = false;

  // Use the optimized DrawCommand that takes a dirty rect
  DrawCommand *cmd = new DrawCommand(canvas.drawingSurface, Boundbox);

  // Finalize Profiler
  Profiler::commitRace({{"BRESENHAM", ImVec4(1.0f, 0.5f, 0.0f, 1.0f)},
                        {"DDA", ImVec4(0.0f, 0.5f, 1.0f, 1.0f)}});

  Logger::log(LogLevel::DEBUG, "ERASER STOPPED DRAWING");
  return cmd;
}
