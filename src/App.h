#pragma once
#include "../Tools/ToolManager.h"
#include "Canvas.h"
#include "CommandManager.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <vector>
class App {
public:
  App(const char *title);
  ~App();

  void run();

  static inline float frameTimes[100] = {0};
  static inline int frameOffset = 0;

private:
  bool initSDL(const char *title);
  void initImGui();
  void cleanup();
  void processInput(bool &running, ToolManager &tm, Canvas &canvas,
                    CommandManager &cm, SDL_Window *window);
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  Canvas *canvas = nullptr;
  CommandManager cm;
  ToolManager tm;

  bool running = true;
  int screenW, screenH;
};
