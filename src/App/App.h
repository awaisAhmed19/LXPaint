#pragma once
#include <SDL3/SDL.h>

#include <memory>

#include "App/Globals.h"
#include "Window.h"
#include "imgui_impl_sdl3.h"

#include "Editor/Editor.h"
#include "UI/ColorPalette.h"
#include "UI/Footer.h"
#include "UI/LayoutEngine/LayoutEngine.h"
#include "UI/Ribbon.h"
#include "UI/Toolbar.h"

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
  std::unique_ptr<LayoutEngine> m_layoutEngine = nullptr;
  std::unique_ptr<UI::Ribbon> m_ribbon = nullptr;
  std::unique_ptr<UI::Toolbar> m_toolbar = nullptr;
  std::unique_ptr<UI::ColorPalette> m_colorpalette = nullptr;
  std::unique_ptr<UI::Footer> m_footer = nullptr;
  ToolType m_lastTool;

private:
  void handleEvents();
  void update();
  void render();
};
}; // namespace App
