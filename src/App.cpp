#include "App.h"
#include "../Tools/Eraser.h"
#include "../Tools/Line.h"
#include "../Tools/Pencil.h"
#include "../UI/Console.h"
#include "Logger.h"

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

  tm.registerTool("pencil", std::make_unique<Pencil>());
  tm.registerTool("line", std::make_unique<Line>());
  tm.registerTool("eraser", std::make_unique<Eraser>());
  tm.setActiveTool("line");

  // ImGui Setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);
}

void App::handleEvents() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    ImGui_ImplSDL3_ProcessEvent(&e);

    // 1. System Events
    if (e.type == SDL_EVENT_QUIT) {
      running = false;
      return;
    }

    // 2. Keyboard Shortcuts (Abstraction: handleShortcuts)
    if (e.type == SDL_EVENT_KEY_DOWN) {
      if (e.key.mod & SDL_KMOD_CTRL) {
        switch (e.key.key) {
        case SDLK_Z:
          cm.undo(*canvas);
          break;
        case SDLK_Y:
          cm.redo(*canvas);
          break;
        case SDLK_E:
          tm.setActiveTool("eraser");
          break;
        case SDLK_P:
          tm.setActiveTool("pencil");
          break;
        case SDLK_L:
          tm.setActiveTool("line");
          break;
        }
        // CRITICAL: Stop processing this event so it doesn't leak into mouse
        // logic
        continue;
      }
    }

    // 3. Mouse Interaction (UI Guard)
    if (ImGui::GetIO().WantCaptureMouse)
      continue;

    // 4. Tool Execution
    BaseTool *tool = tm.getActive();
    if (!tool)
      continue;

    switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        // Coordinates are only valid during MOUSE events
        tool->onMouseDown({(float)e.button.x, (float)e.button.y}, *canvas);
      }
      break;

    case SDL_EVENT_MOUSE_MOTION:
      if (e.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
        tool->onMouseMove({(float)e.motion.x, (float)e.motion.y}, *canvas);
      }
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        auto cmd =
            tool->onMouseUp({(float)e.button.x, (float)e.button.y}, *canvas);
        if (cmd)
          cm.executeCommand(cmd, *canvas);
      }
      break;
    }
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

  // Render Canvas (CPU -> GPU)
  canvas->syncTexture();
  SDL_FRect dest = {0, 0, (float)canvas->w, (float)canvas->h};
  SDL_RenderTexture(renderer, canvas->mainTexture, NULL, &dest);

  // Render UI Overlays (Consoles, Benchmarks)
  DrawLogConsole(*canvas, screenW, screenH);

  // Finalize
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
  SDL_RenderPresent(renderer);
}

void App::run() {
  while (running) {
    handleEvents();
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
