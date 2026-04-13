#include <SDL3/SDL.h>
#include <iostream>
#include "./Tools/ToolManager.h"
#include "./Tools/Pencil.h"
#include "./core/CommandManager.h"
#include "./core/Canvas.h"
#include "Globals.h"

const char *TITLE = "LXPAINT";
int WindowWidth = 1280;
int WindowHeight = 720;
void processInput(bool& running, ToolManager& tm, Canvas& canvas, CommandManager& cm) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {

        BaseTool* tool = tm.getActive();
        if (!tool) continue;
      switch (e.type) {
        case SDL_EVENT_QUIT:
            running = false;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_Z && (e.key.mod & SDL_KMOD_CTRL)) {
                // CommandManager should handle undo, not the tool!
                cm.undo(canvas);
            }
            if (e.key.key == SDLK_Y && (e.key.mod & SDL_KMOD_CTRL)) {
                // CommandManager should handle undo, not the tool!
                cm.redo(canvas);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (e.button.button == SDL_BUTTON_LEFT) {
                vec2 mousePos = { (int)e.button.x, (int)e.button.y };
                tool->onMouseDown(mousePos, canvas);
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            if (e.motion.state & SDL_BUTTON_LMASK) {
                vec2 mousePos = { (int)e.motion.x, (int)e.motion.y };
                tool->onMouseMove(mousePos, canvas);
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (e.button.button == SDL_BUTTON_LEFT) {
                vec2 mousePos = { (int)e.button.x, (int)e.button.y };
                Command* cmd = tool->onMouseUp(mousePos, canvas);
                if (cmd) cm.executeCommand(cmd,canvas);
            }
            break;
    }

    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    // 1. Create Window and Renderer FIRST
    if (!SDL_CreateWindowAndRenderer(TITLE, WindowWidth, WindowHeight, 0, &window, &renderer)) {
        SDL_Quit();
        return 1;
    }

    // 2. Initialize Engine Systems
    Canvas canvas(renderer, WindowWidth, WindowHeight);
    CommandManager cm;
    ToolManager tm;

    // 3. Register and Setup Tools
    Pencil* pencil = new Pencil();
    tm.registerTool("pencil", pencil);
    tm.setActiveTool("pencil");

    bool running = true;
    while (running) {
        // A. Handle Input
        processInput(running, tm, canvas, cm);

        // B. Update Display Texture from Software Surface
        canvas.syncTexture(); // Assumes you added this helper to Canvas.h

        // C. Render
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Background color
        SDL_RenderClear(renderer);

        // Draw the canvas texture to the screen
        SDL_RenderTexture(renderer, canvas.mainTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    delete pencil;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
