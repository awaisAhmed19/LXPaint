#include "App.h"
#include "../UI/Console.h"
App::App(const char *title) {
  SDL_Init(SDL_INIT_VIDEO);

  // Get Display Bounds
  SDL_DisplayID id = SDL_GetPrimaryDisplay();
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(id);
  screenW = mode->w;
  screenH = mode->h;

  window = SDL_CreateWindow(title, screenW, screenH, SDL_WINDOW_FULLSCREEN);
  renderer = SDL_CreateRenderer(window, NULL);

  // Initialize Canvas & Tools
  canvas = std::make_unique<Canvas>(renderer, (int)(screenW * 0.7f),
                                    (int)(screenH * 0.7f));
  cW = canvas->getWidth();
  cH = canvas->getHeight();
  tm.registerTool("pencil", std::make_unique<Pencil>());
  tm.registerTool("line", std::make_unique<Line>());
  tm.registerTool("rect", std::make_unique<Rect>());
  tm.registerTool("eraser", std::make_unique<Eraser>());
  tm.setActiveTool("pencil");
  tool = tm.getActive();
  // ImGui Setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
}

void App::mouseEvents(SDL_Event e) {
  tool = tm.getActive();
  if (!tool) {
    Logger::log(LogLevel::DEBUG, "TOOL IS NULL MOSTLY LIKELY");
    return;
  }
  switch (e.type) {
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    if (e.button.button == SDL_BUTTON_LEFT &&
        inCanvas(e.button.x, e.button.y)) {
      // Coordinates are only valid during MOUSE events
      tool->onMouseDown({(float)e.button.x, (float)e.button.y}, *canvas);
    }
    break;

  case SDL_EVENT_MOUSE_MOTION:
    if (e.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT) &&
        inCanvas(e.motion.x, e.motion.y)) {
      tool->onMouseMove({(float)e.motion.x, (float)e.motion.y}, *canvas);
    }
    break;

  case SDL_EVENT_MOUSE_BUTTON_UP:
    if (e.button.button == SDL_BUTTON_LEFT &&
        inCanvas(e.button.x, e.button.y)) {
      auto cmd =
          tool->onMouseUp({(float)e.button.x, (float)e.button.y}, *canvas);
      if (cmd)
        cm.executeCommand(cmd, *canvas);
    }
    break;
  }
}

void App::handleEvents(SDL_Event e) {
  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL3_ProcessEvent(&e);
    // 1. System Events
    if (e.type == SDL_EVENT_QUIT) {
      running = false;
      return;
    }
    // 2. Keyboard Shortcuts (Abstraction: handleShortcuts)
    dispatcher.updateKeyInput(e);
    // 3. Mouse Interaction (UI Guard)
    if (ImGui::GetIO().WantCaptureMouse)
      continue;
    mouseEvents(e);
    // 4. Tool Execution
    continue;
  }
}
void App::render() {
  // Start Frame
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // Clear Screen
  SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
  SDL_RenderClear(renderer);

  // --- MAIN CANVAS ---
  canvas->syncTexture();

  SDL_FRect dest = {0, 0, (float)canvas->getWidth(),
                    (float)canvas->getHeight()};
  SDL_RenderTexture(renderer, canvas->m_canvasTexture, NULL, &dest);

  SDL_RenderTexture(renderer, canvas->m_previewTexture, NULL, &dest);

  // --- UI ---
  DrawLogConsole(*canvas, screenW, screenH, frameTimes, frameOffset);

  // Finalize
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}

void App::setupInputKeyBinds() {
  dispatcher.keyBinds(SDL_SCANCODE_Z, InputCommand::UNDO);
  dispatcher.keyBinds(SDL_SCANCODE_Y, InputCommand::REDO);
  dispatcher.keyBinds(SDL_SCANCODE_P, InputCommand::PENCIL);
  dispatcher.keyBinds(SDL_SCANCODE_F, InputCommand::FILL);
  dispatcher.keyBinds(SDL_SCANCODE_L, InputCommand::LINE);
  dispatcher.keyBinds(SDL_SCANCODE_R, InputCommand::RECT);
  dispatcher.keyBinds(SDL_SCANCODE_E, InputCommand::ERASER);
}

void App::setupInputActions() {
  dispatcher.bindActions(InputCommand::UNDO, [this]() { cm.undo(*canvas); });
  dispatcher.bindActions(InputCommand::REDO, [this]() { cm.undo(*canvas); });
  dispatcher.bindActions(InputCommand::PENCIL,
                         [this]() { tm.setActiveTool("pencil"); });
  dispatcher.bindActions(InputCommand::FILL,
                         [this]() { tm.setActiveTool("fill"); });
  dispatcher.bindActions(InputCommand::LINE,
                         [this]() { tm.setActiveTool("line"); });
  dispatcher.bindActions(InputCommand::RECT,
                         [this]() { tm.setActiveTool("rect"); });
  dispatcher.bindActions(InputCommand::ERASER,
                         [this]() { tm.setActiveTool("eraser"); });
}

void App::run() {
  setupInputKeyBinds();
  setupInputActions();
  while (running) {
    handleEvents(events);
    render();
  }
}

App::~App() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
