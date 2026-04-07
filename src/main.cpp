#include <SDL3/SDL.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // 1. Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Create Window and Renderer together
    // SDL_CreateWindowAndRenderer is the fastest way to get both.
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer("SDL3 Window", 640, 480, 0, &window, &renderer)) {
        std::cerr << "Failed to create window/renderer: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    // 3. Main Loop
    while (running) {
        // Event Handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // --- Rendering ---
        // Set draw color (R, G, B, A) -> Cornflower Blue
        SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);

        // Clear the screen with that color
        SDL_RenderClear(renderer);

        // Present the backbuffer to the window
        SDL_RenderPresent(renderer);
    }

    // 4. Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
