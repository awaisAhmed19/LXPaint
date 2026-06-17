#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

#include "App.h"

#include "App/Globals.h"
#include "imgui_impl_sdlrenderer3.h"

#include "Systems/Logger.h"

namespace App {
Application::Application(const char *title) : m_editor(nullptr) {
  unsigned int init_flags = SDL_INIT_VIDEO;
  Logger::init();
  if (!SDL_Init(init_flags)) {
    Logger::err(SDL_GetError());
    throw std::runtime_error("SDL_Init failed");
  }

  this->m_window = std::make_unique<Window>(Window::Settings{"LXPAINT"});
  this->m_editor = std::make_unique<Editor>(m_window->getNativeRenderer());
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  this->m_ribbon = std::make_unique<UI::Ribbon>(1300, 10);
  this->m_toolbar = std::make_unique<UI::Toolbar>(0, 0);
  this->m_toolbar->init(this->m_window->getNativeRenderer());
  this->m_colorpallete = std::make_unique<UI::ColorPallete>(1300, 0);
  ImGui_ImplSDL3_InitForSDLRenderer(m_window->getNativeWindow(),
                                    m_window->getNativeRenderer());
  ImGui_ImplSDLRenderer3_Init(m_window->getNativeRenderer());

  // LXTheme::applyMainTheme();
}

void Application::handleEvents() {
  while (SDL_PollEvent(&this->m_event)) {
    ImGui_ImplSDL3_ProcessEvent(&this->m_event);
    if (this->m_event.type == SDL_EVENT_QUIT) {
      this->m_running = false;
      return;
    }
    if (ImGui::GetIO().WantCaptureMouse)
      continue;
    this->m_editor->handleEvent(this->m_event);
  }
}
void Application::render() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();

  ImGui::NewFrame();

  SDL_SetRenderDrawColor(m_window->getNativeRenderer(), 128, 128, 128, 255);

  SDL_RenderClear(m_window->getNativeRenderer());

  m_editor->render();
  m_editor->renderUI();

  m_ribbon->render();
  m_toolbar->render();
  m_colorpallete->render();
  m_editor->setFgColor(UI::ColorPallete::toU32(m_colorpallete->getFgColor()));
  m_editor->setBgColor(UI::ColorPallete::toU32(m_colorpallete->getBgColor()));
  ImGui::Render();

  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        m_window->getNativeRenderer());

  SDL_RenderPresent(m_window->getNativeRenderer());
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
  SDL_Quit();
}
}; // namespace App
