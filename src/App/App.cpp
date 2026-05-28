#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

#include "App.h"

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
  ImGui_ImplSDL3_InitForSDLRenderer(m_window->getNativeWindow(),
                                    m_window->getNativeRenderer());
  ImGui_ImplSDLRenderer3_Init(m_window->getNativeRenderer());
}

void Application::handleEvents() {
  while (SDL_PollEvent(&this->m_event)) {
    ImGui_ImplSDL3_ProcessEvent(&this->m_event);
    // 1. System Events
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
  SDL_SetRenderDrawColor(m_window->getNativeRenderer(), 128, 128, 128,
                         255); // background color of app
  SDL_RenderClear(m_window->getNativeRenderer());
  m_editor->render();
  m_editor->renderUI();

  // DrawLogConsole(m_editor->getCanvas(), m_screenW, m_screenH, frameTimes,
  //                frameOffset);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),
                                        m_window->getNativeRenderer());
  SDL_RenderPresent(m_window->getNativeRenderer());
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
    m_editor->update();
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
