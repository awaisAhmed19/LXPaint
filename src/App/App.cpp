#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

#include "App.h"
#include "App/Globals.h"
#include "Editor/Tools/Text.h"
#include "Systems/Logger.h"
#include "UI/LayoutEngine/LayoutMetrics.h"
#include "imgui_impl_sdlrenderer3.h"
#include <iostream>

namespace App {
Application::Application(const char *title) : m_editor(nullptr) {
  unsigned int init_flags = SDL_INIT_VIDEO;
  Logger::init();
  if (!SDL_Init(init_flags)) {
    Logger::err(SDL_GetError());
    throw std::runtime_error("SDL_Init failed");
  }
  if (!TTF_Init()) {
    Logger::err(SDL_GetError());
    throw std::runtime_error("TTF_Init failed");
  }
  if (!Text::initFontSystem("../assets/fonts/NotoSans-Regular.ttf", 16)) {
    Logger::err(
        "Text font failed to load — text tool will be visually disabled");
    // not fatal: Text degrades gracefully per the null-s_font guards above
  }
  this->m_window = std::make_unique<Window>(Window::Settings{"LXPAINT"});

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto size = m_window->size();

  m_layoutEngine = std::make_unique<LayoutEngine>();
  this->m_ribbon = std::make_unique<UI::Ribbon>(size.width, 10);
  this->m_toolbar =
      std::make_unique<UI::Toolbar>(0, 0, m_window->nativeRenderer());
  this->m_toolbar->init();
  this->m_colorpalette = std::make_unique<UI::ColorPalette>(size.width, 0);
  this->m_footer = std::make_unique<UI::Footer>(size.width, 0);
  m_layoutEngine->update(size.width, size.height);
  auto vp = m_layoutEngine->layout().viewport;

  std::cout << "VP INFO FROM APP" << vp.x << " " << vp.y << " " << vp.width
            << " " << vp.height << '\n';

  this->m_editor = std::make_unique<Editor>(m_window->nativeWindow(),
                                            m_window->nativeRenderer(),
                                            m_layoutEngine->layout());

  ImGui_ImplSDL3_InitForSDLRenderer(m_window->nativeWindow(),
                                    m_window->nativeRenderer());
  ImGui_ImplSDLRenderer3_Init(m_window->nativeRenderer());
}

void Application::handleEvents() {
  while (SDL_PollEvent(&m_event)) {

    // Give ImGui first chance to process the event
    ImGui_ImplSDL3_ProcessEvent(&m_event);

    switch (m_event.type) {

    case SDL_EVENT_QUIT:
      m_running = false;
      return;

    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
      auto size = m_window->size();

      m_layoutEngine->update(size.width, size.height);

      const auto &layout = m_layoutEngine->layout();

      m_editor->setViewportRect({
          layout.viewport.x,
          layout.viewport.y,
          layout.viewport.width,
          layout.viewport.height,
      });
      std::cout << "\n=== WINDOW RESIZED ===\n";
      std::cout << "Window: " << size.width << " x " << size.height << '\n';

      std::cout << "Viewport: " << layout.viewport.x << ", "
                << layout.viewport.y << ", " << layout.viewport.width << ", "
                << layout.viewport.height << '\n';
      // Uncomment if you want automatic fit-to-window on resize.
      // m_editor->fitCanvasToScreen();

      break;
    }

    default:
      break;
    }

    // Don't forward mouse/keyboard events to the editor if ImGui wants them.
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
      continue;
    }

    m_editor->handleEvent(m_event);
  }
}
void Application::render() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();

  ImGui::NewFrame();

  SDL_SetRenderDrawColor(m_window->nativeRenderer(), 128, 128, 128, 255);

  SDL_RenderClear(m_window->nativeRenderer());

  m_editor->render();
  m_editor->renderUI();

  m_ribbon->render();
  m_toolbar->render(*m_editor);
  m_colorpalette->render();
  m_footer->render();
  m_editor->setFgColor(UI::ColorPalette::toU32(m_colorpalette->getFgColor()));
  m_editor->setBgColor(UI::ColorPalette::toU32(m_colorpalette->getBgColor()));
  ImGui::Render();

  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        m_window->nativeRenderer());

  SDL_RenderPresent(m_window->nativeRenderer());
}

void Application::update() {
  ToolType currentTool = m_toolbar->getActiveTool();

  if (currentTool != m_lastTool) {
    m_editor->setActiveTool(currentTool);
    m_lastTool = currentTool;
  }

  m_editor->update();
}

int Application::run() {
  uint64_t last = SDL_GetTicks();
  if (m_exist_status == 1) {
    return m_exist_status;
  }
  while (m_running) {
    uint64_t now = SDL_GetTicks();
    float deltaTime = (now - last) / 1000.0f;
    last = now;
    handleEvents();
    update();
    render();
  }
  return m_exist_status;
}
void Application::stop() { m_running = false; }
Application::~Application() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  Text::shutdownFontSystem();
  TTF_Quit();
  SDL_Quit();
}
}; // namespace App
