#include <SDL.h>

#include <iostream>

using namespace std;

SDL_Renderer* renderer = 0;
SDL_Window* window = 0;
bool init(const char* Title, int Xpos, int Ypos, int height, int width, int flags) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(Title, Xpos, Ypos, width, height, flags);
    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    bool running = false;

    if (init("LXPaint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 650, 650,
             SDL_WINDOW_SHOWN)) {
        running = true;
        std::cout << running;
    } else {
        return 1;
    }

    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        Render();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
