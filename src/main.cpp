#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <iostream>
// https://stackoverflow.com/questions/19935727/sdl2-how-to-render-with-one-buffer-instead-of-two
#include "canvas.h"
SDL_Window* win = nullptr;
SDL_Renderer* renderer = nullptr;

bool initWindow() {
    win = SDL_CreateWindow("LXPaint", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1270, 720,
                           SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!win) {
        std::cout << "Error initializing window: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool initRenderer() {
    renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "Error initializing Surface: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    return initWindow() && initRenderer();
}

void kill() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    renderer = nullptr;
    win = nullptr;
    SDL_Quit();
}

void render(uint32_t* buffer, SDL_Texture* texture) {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    // do drawing in this
    if (!init()) return 1;
    Canvas c;
    c = createCanvas(100, 100);
    fillWhite(c);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,  // Or your preferred format
                                             SDL_TEXTUREACCESS_STREAMING, c.width, c.height);

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        SDL_UpdateTexture(texture, NULL, c.buffer.data(), c.width * 4);
        render(c.buffer.data(), texture);
    }
    SDL_DestroyTexture(texture);
    kill();
    return 0;
}
