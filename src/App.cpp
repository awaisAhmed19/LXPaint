#include "../Tools/FloodFill.h"
#include "../Tools/Line.h"
#include "../Tools/Pencil.h"
#include "../UI/Console.h"
#include "App.h"
#include "Logger.h"

App::App(const char* title) {
    if (!initSDL(title)) return;
    initImGui();

    // Setup Canvas (70% logic)
    int canvasW = (screenW * 0.7f);
    int canvasH = (screenH * 0.7f);
    canvas = new Canvas(renderer, canvasW, canvasH);
    // Setup Tools
    std::string tool = "floodfill";
    tm.registerTool("pencil", new Pencil());
    tm.registerTool("floodfill", new FloodFill());
    tm.registerTool("line", new Line());
    tm.setActiveTool(tool);
}

void App::processInput(bool& running, ToolManager& tm, Canvas& canvas, CommandManager& cm,
                       SDL_Window* window) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        // 1. Let ImGui see the events first (Crucial for the Console UI)
        ImGui_ImplSDL3_ProcessEvent(&e);

        BaseTool* tool = tm.getActive();
        if (!tool) continue;

        switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_KEY_DOWN:
                // Fullscreen toggle
                if (e.key.key == SDLK_F1) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    SDL_SetWindowFullscreen(window, !(flags & SDL_WINDOW_FULLSCREEN));
                }

                // Undo / Redo / Clear
                if (e.key.mod & SDL_KMOD_CTRL) {
                    if (e.key.key == SDLK_Z) cm.undo(canvas);
                    if (e.key.key == SDLK_Y) cm.redo(canvas);
                    if (e.key.key == SDLK_C) canvas.clearAll();

                    // ALGO SWITCHING (Using the global we defined in Globals.h)
                    if (e.key.key == SDLK_1) {
                        g_CurrentAlgo = LineAlgo::BRESENHAM;
                        Logger::log(LogLevel::INFO, "Switched to BRESENHAM");
                    }
                    if (e.key.key == SDLK_2) {
                        g_CurrentAlgo = LineAlgo::DDA;
                        Logger::log(LogLevel::INFO, "Switched to DDA");
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Added ImGui check: Don't draw if clicking on the Console UI
                    if (!ImGui::GetIO().WantCaptureMouse && e.button.x <= canvas.w &&
                        e.button.y <= canvas.h) {
                        tool->onMouseDown({e.button.x, e.button.y}, canvas);
                    }
                }
                break;

            case SDL_EVENT_MOUSE_MOTION: {
                float posX = e.motion.x;
                float posY = e.motion.y;

                if (e.motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
                    if (!ImGui::GetIO().WantCaptureMouse && posX <= (float)canvas.w &&
                        posY <= (float)canvas.h) {
                        tool->onMouseMove({posX, posY}, canvas);
                    }
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    Command* cmd = tool->onMouseUp({e.button.x, e.button.y}, canvas);
                    if (cmd) cm.executeCommand(cmd, canvas);
                }
                break;

        }  // End switch (e.type)
    }  // End while (SDL_PollEvent)
}
bool App::initSDL(const char* title) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return false;

    SDL_DisplayID id = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(id);
    screenW = mode->w;
    screenH = mode->h;

    window = SDL_CreateWindow(title, screenW, screenH, SDL_WINDOW_FULLSCREEN);
    renderer = SDL_CreateRenderer(window, NULL);
    return (window && renderer);
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void App::run() {
    while (running) {
        auto startFrame = std::chrono::high_resolution_clock::now();
        processInput(running, tm, *canvas, cm, window);

        // 2. CPU -> GPU Data Transfer
        canvas->syncTexture();

        // 3. ImGui Lifecycle
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 4. UI Rendering Logic
        DrawLogConsole(*canvas, screenW, screenH);

        // 5. Scene Rendering
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderClear(renderer);

        // Draw the Paper/Canvas
        SDL_FRect dest = {0, 0, (float)canvas->w, (float)canvas->h};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &dest);
        SDL_RenderTexture(renderer, canvas->mainTexture, NULL, &dest);

        // 6. Overlay ImGui
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);

        // 7. Swap Buffers
        SDL_RenderPresent(renderer);

        // --- END TOTAL FRAME TIMER ---
        auto endFrame = std::chrono::high_resolution_clock::now();
        float duration = std::chrono::duration<float, std::milli>(endFrame - startFrame).count();
        float currentFPS = (duration > 0.0f) ? (1000.0f / duration) : 0.0f;
        // Record to Circular Buffer
        App::frameTimes[App::frameOffset] = duration;
        App::frameOffset = (App::frameOffset + 1) % 100;
    }
}
App::~App() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    delete canvas;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
