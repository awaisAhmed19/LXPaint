#pragma once
#include <cstdint>
class Canvas;
class PreviewLayer;

struct ToolInteractionState;
struct ToolSettings;
class CommandManager;

struct ToolContext {
  Canvas *canvas;
  PreviewLayer *preview;
  ToolInteractionState *interaction;

  CommandManager *commandManager;

  uint32_t fgColor;
  uint32_t bgColor;
  int brushSize = 1;
  ToolSettings *settings;
};
