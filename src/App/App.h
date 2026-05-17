#pragma once
#include "../Editor/Editor.h"
#include "../Systems/Logger.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
constexpr int DEFAULT_WINDOW_WIDTH = 1280;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;
class App {
public:
  App(const char *title);
  ~App();

  void run();

  static inline float frameTimes[100] = {0};
  static inline int frameOffset = 0;

private:
  bool m_running = true;
  int m_screenW = 0, m_screenH = 0;

  SDL_Window *m_window = nullptr;
  SDL_Renderer *m_renderer = nullptr;
  SDL_Event m_event;
  std::unique_ptr<Editor> m_editor;

private:
  void handleEvents();
  void update(float deltatime);
  void render();
  // bool inCanvas(float x, float y) {
  //   if ((x > 0 && x < cW) && (y > 0 && y < cH))
  //     return true;
  //   else
  //     return false;
  // }
};
