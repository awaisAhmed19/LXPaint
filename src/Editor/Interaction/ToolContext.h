#pragma once

class Canvas;
class PreviewLayer;

struct ToolInteractionState;
struct ToolContext {
  Canvas *canvas;
  PreviewLayer *preview;
  ToolInteractionState *interaction;
};
