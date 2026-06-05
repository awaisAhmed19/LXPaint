#pragma once
#include <cstdint>
class Canvas;
class PreviewLayer;

struct ToolInteractionState;
struct ToolContext {
  Canvas *canvas;
  PreviewLayer *preview;
  ToolInteractionState *interaction;
  uint32_t color = 0;
};
