#pragma once
#include <cstdint>
class Canvas;
class PreviewLayer;

struct ToolInteractionState;
struct ToolSettings;
struct ToolContext {
  Canvas *canvas;
  PreviewLayer *preview;
  ToolInteractionState *interaction;
  uint32_t fgColor;
  uint32_t bgColor;
  int brushSize = 1;
  ToolSettings *settings;
};
