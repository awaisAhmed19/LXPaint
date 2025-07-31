#include <SDL2/SDL.h>

#include <iostream>

#include "../include/Canvas.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_init Error" << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("MS Paint", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       1280, 720, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cerr << "SDL_init Error" << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Renderer* ren =
        SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    Canvas Canvas(ren, 0, 0, 800, 600);
    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
