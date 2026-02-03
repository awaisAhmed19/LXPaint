#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include <iostream>

SDL_Window* win = nullptr;
SDL_Surface* winSurface = nullptr;
SDL_Surface* img1;
SDL_Surface* img2;
bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    win = SDL_CreateWindow("LXPaint", 0, 0, 1270, 800, SDL_WINDOW_SHOWN);
    if (!win) {
        std::cout << "Error initializing window: " << SDL_GetError() << std::endl;
        return false;
    }
    winSurface = SDL_GetWindowSurface(win);

    if (!winSurface) {
        std::cout << "Error initializing Surface: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}
void kill() {
    SDL_FreeSurface(img1);
    SDL_FreeSurface(img2);
    SDL_DestroyWindow(win);
    win = nullptr;
    winSurface = nullptr;
    SDL_Quit();
}
bool load() {
    SDL_Surface *temp1, *temp2;

    // Load images
    // Change these lines in your load() function
    temp1 = SDL_LoadBMP("src/sample2.bmp");
    temp2 = SDL_LoadBMP("src/sample3.bmp");

    // Make sure loads succeeded
    if (!temp1 || !temp2) {
        std::cout << "Error loading image: " << SDL_GetError() << std::endl;
        system("pause");
        return false;
    }

    // Format surfaces
    img1 = SDL_ConvertSurface(temp1, winSurface->format, 0);
    img2 = SDL_ConvertSurface(temp2, winSurface->format, 0);

    // Free temporary surfaces
    SDL_FreeSurface(temp1);
    SDL_FreeSurface(temp2);

    // Make sure format succeeded
    if (!img1 || !img2) {
        std::cout << "Error converting surface: " << SDL_GetError() << std::endl;
        system("pause");
        return false;
    }
    return true;
}
int main(int argc, char* argv[]) {
    // do drawing in this

    if (!init()) return 1;

    if (!load()) return 1;

    // Blit image to entire window
    SDL_BlitSurface(img1, NULL, winSurface, NULL);

    // Blit image to scaled portion of window
    SDL_Rect dest;
    dest.x = 160;
    dest.y = 120;
    dest.w = 320;
    dest.h = 240;
    SDL_BlitScaled(img2, NULL, winSurface, &dest);
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
        // You can keep the update here if you're animating
        SDL_UpdateWindowSurface(win);
    }

    kill();
    return 0;
}
