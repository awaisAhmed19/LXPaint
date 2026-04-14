#include "./Tools/Pencil.h"
#include "./Tools/ToolManager.h"
#include "./core/Canvas.h"
#include "./core/CommandManager.h"
#include "./core/Logger.h" // Assuming you added the Logger
#include "Globals.h"
#include <SDL3/SDL.h>
#include <iostream>

const char *TITLE = "LXPAINT";

void processInput(bool &running, ToolManager &tm, Canvas &canvas,
                  CommandManager &cm, SDL_Window *window) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    BaseTool *tool = tm.getActive();
    if (!tool)
      continue;

    switch (e.type) {
    case SDL_EVENT_QUIT:
      running = false;
      break;

    case SDL_EVENT_KEY_DOWN:
      // Fullscreen toggle for debugging (F1)
      if (e.key.key == SDLK_F1) {
        Uint32 flags = SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window, !(flags & SDL_WINDOW_FULLSCREEN));
      }
      // Undo / Redo logic
      if (e.key.key == SDLK_Z && (e.key.mod & SDL_KMOD_CTRL))
        cm.undo(canvas);
      if (e.key.key == SDLK_Y && (e.key.mod & SDL_KMOD_CTRL))
        cm.redo(canvas);
      break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        if (e.button.x <= canvas.w && e.button.y <= canvas.h) {
          Logger::log(LogLevel::INFO, "SDL_BUTTON_LEFT CLICKED ACTIVATED");
          std::cout << "canvas width and height" << canvas.w << canvas.h;
          tool->onMouseDown({(int)e.button.x, (int)e.button.y}, canvas);
        }
      }
      break;

    case SDL_EVENT_MOUSE_MOTION:
      // FIX: SDL3 uses SDL_BUTTON_MASK for bitwise checks on motion state
      if (e.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
        if (e.motion.x <= canvas.w && e.motion.y <= canvas.h) {
          Logger::log(LogLevel::INFO, "SDL_BUTTON_LEFT MOTION ACTIVATED");
          std::cout << "canvas width and height" << canvas.w << canvas.h;
          tool->onMouseMove({(int)e.motion.x, (int)e.motion.y}, canvas);
        }
      }
      break;

    case SDL_EVENT_MOUSE_BUTTON_UP:
      if (e.button.button == SDL_BUTTON_LEFT) {
        // We don't bound check on Up because a user might drag off the canvas
        // and release
        Logger::log(LogLevel::INFO, "SDL_BUTTON_LEFT BUTTON UP");
        std::cout << "canvas width and height" << canvas.w << canvas.h;
        Command *cmd =
            tool->onMouseUp({(int)e.button.x, (int)e.button.y}, canvas);
        if (cmd)
          cm.executeCommand(cmd, canvas);
      }
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    Logger::log(LogLevel::ERR, "SDL_Init Failed");
    return 1;
  }

  // Get display stats for layout
  SDL_DisplayID displayID = SDL_GetPrimaryDisplay();
  const SDL_DisplayMode *mode = SDL_GetCurrentDisplayMode(displayID);
  if (!mode)
    return 1;

  int screenW = mode->w;
  int screenH = mode->h;

  // Create window - Note: Hyprland likes SDL_WINDOW_RESIZABLE for tiling
  SDL_Window *window =
      SDL_CreateWindow(TITLE, screenW, screenH, SDL_WINDOW_FULLSCREEN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  if (!window || !renderer) {
    Logger::log(LogLevel::ERR, "Window/Renderer Creation Failed");
    return 1;
  }

  // Canvas setup (70% of screen)
  int canvasW = (int)(screenW * 0.7f);
  int canvasH = (int)(screenH * 0.7f);
  SDL_FRect canvasDest = {0.0f, 0.0f, (float)canvasW, (float)canvasH};

  Canvas canvas(renderer, canvasW, canvasH);
  CommandManager cm;
  ToolManager tm;

  // Register Tools
  Pencil *pencil = new Pencil();
  tm.registerTool("pencil", pencil);
  tm.setActiveTool("pencil");

  Logger::log(LogLevel::INFO,
              "Engine Initialized. Running on Workspace 2 (hopefully).");

  bool running = true;
  while (running) {
    // A. Input Handling
    processInput(running, tm, canvas, cm, window);

    // B. Logic/Sync
    canvas.syncTexture();

    // C. Rendering
    // 1. Clear background to Grey
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);

    // 2. Draw white "Paper" sheet
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &canvasDest);

    // 3. Render the drawing texture onto the white sheet
    SDL_RenderTexture(renderer, canvas.mainTexture, NULL, &canvasDest);

    SDL_RenderPresent(renderer);
  }

  // Cleanup
  delete pencil;
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
