#pragma once
#include <SDL3/SDL.h>

#include <memory>

#include "Window.h"
#include "imgui_impl_sdl3.h"

#include "Editor/Editor.h"

namespace App {

constexpr int DEFAULT_WINDOW_WIDTH = 1280;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;

class Application {
public:
  Application(const char *title);
  ~Application();

  int run();
  void stop();

private:
  bool m_running = true;
  int m_exist_status = 0;
  SDL_Event m_event;
  std::unique_ptr<Editor> m_editor = nullptr;
  std::unique_ptr<Window> m_window = nullptr;

private:
  void handleEvents();
  void update(float deltatime);
  void render();
};
}; // namespace App
