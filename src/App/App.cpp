#include "App.h"
#include "../UI/Console.h"
App::App(const char *title) : m_editor(nullptr) {
  SDL_Init(SDL_INIT_VIDEO);

  // Get Display Bounds
  SDL_DisplayID id = SDL_GetPrimaryDisplay();
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(id);
  this->m_screenW = mode->w;
  this->m_screenH = mode->h;

  this->m_window = SDL_CreateWindow(title, this->m_screenW, this->m_screenH,
                                    SDL_WINDOW_FULLSCREEN);
  this->m_renderer = SDL_CreateRenderer(this->m_window, NULL);

  Logger::init();
  this->m_editor = std::make_unique<Editor>(this->m_renderer);
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForSDLRenderer(this->m_window, this->m_renderer);
  ImGui_ImplSDLRenderer3_Init(this->m_renderer);
}

void App::handleEvents() {
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
void App::render() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();

  ImGui::NewFrame();
  SDL_SetRenderDrawColor(m_renderer, 128, 128, 128,
                         255); // background color of app
  SDL_RenderClear(m_renderer);
  m_editor->render();

  // DrawLogConsole(m_editor->getCanvas(), m_screenW, m_screenH, frameTimes,
  //                frameOffset);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer);
  SDL_RenderPresent(m_renderer);
}
void App::run() {
  uint64_t last = SDL_GetTicks();

  while (m_running) {
    uint64_t now = SDL_GetTicks();
    float deltaTime = (now - last) / 1000.0f;
    last = now;
    handleEvents();
    m_editor->update();
    render();
  }
}

App::~App() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(this->m_renderer);
  SDL_DestroyWindow(this->m_window);
  SDL_Quit();
}
