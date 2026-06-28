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

  // Give the editor a pointer to the dialog manager so MenuActionDispatcher
  // can open dialogs via Editor::dialogManager() (see Editor.h).
  m_editor->setDialogManager(&m_dialogManager);

  ImGui_ImplSDL3_InitForSDLRenderer(m_window->nativeWindow(),
                                    m_window->nativeRenderer());
  ImGui_ImplSDLRenderer3_Init(m_window->nativeRenderer());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Event handling
// ─────────────────────────────────────────────────────────────────────────────

void Application::handleEvents() {
  while (SDL_PollEvent(&m_event)) {
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
      m_editor->setViewportRect({layout.viewport.x, layout.viewport.y,
                                 layout.viewport.width,
                                 layout.viewport.height});
      break;
    }

    default:
      break;
    }

    // ── Modal guard
    // ─────────────────────────────────────────────────────────── While a
    // dialog is open, neither ImGui panels nor the editor receive events — the
    // dialog manager has already consumed them via SetNextFrameWantCaptureMouse
    // / SetNextFrameWantCaptureKeyboard.
    if (m_dialogManager.isModal())
      continue;

    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)
      continue;

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

  m_ribbon->render(*m_editor);

  if (m_editor->isToolboxVisible())
    m_toolbar->render(*m_editor);

  if (m_editor->isPaletteVisible())
    m_colorpalette->render();

  if (m_editor->isStatusBarVisible())
    m_footer->render();

  if (m_editor->wasColorSampled()) {
    m_colorpalette->setFgColor(m_editor->getFgColor(),
                               UI::ColorPalette::HexFormat::AARRGGBB);
    m_colorpalette->setBgColor(m_editor->getBgColor(),
                               UI::ColorPalette::HexFormat::AARRGGBB);
    m_editor->clearColorSampled();
  }
  m_editor->setFgColor(UI::ColorPalette::toU32(m_colorpalette->getFgColor()));
  m_editor->setBgColor(UI::ColorPalette::toU32(m_colorpalette->getBgColor()));

  // ── Dialog rendering — MUST be last so it draws on top of everything ────
  m_dialogManager.render();

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

  while (m_running) {
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

} // namespace App
