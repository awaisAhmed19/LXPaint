#pragma once
#include "../Core/Canvas.h"
#include "../Core/CommandManager.h"
#include "../Core/Logger.h"
#include "../Tools/Eraser.h"
#include "../Tools/Line.h"
#include "../Tools/Pencil.h"
#include "../Tools/Rect.h"
#include "../Tools/ToolManager.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
class App {
public:
  App(const char *title);
  ~App();

  void run();

  static inline float frameTimes[100] = {0};
  static inline int frameOffset = 0;

private:
  bool running = true;
  void handleEvents();
  void update(float deltatime);
  void render();

  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  std::unique_ptr<Canvas> canvas;
  CommandManager cm;
  ToolManager tm;

  int screenW, screenH;
};
