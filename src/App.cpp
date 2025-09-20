#include "App.h"

#include <iostream>

bool App::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    int flags = 0;
    if (fullscreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    } else {
        flags = SDL_WINDOW_SHOWN;
    }
    m_pWindow = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
    if (!m_pWindow) {
        std::cerr << "m_pWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!m_pRenderer) {
        std::cerr << "m_pRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Surface* pTempsurface = IMG_Load("/home/awais/LXpaint/assets/sprites/dog.png");
    if (pTempsurface == nullptr) {
        std::cerr << "m_Tempsurface Error: " << SDL_GetError() << std::endl;
    }
    m_pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pTempsurface);
    SDL_FreeSurface(pTempsurface);

    SDL_QueryTexture(m_pTexture, nullptr, nullptr, &m_srcRect.w, &m_srcRect.h);
    m_desRect.x = m_srcRect.x = 0;
    m_desRect.y = m_srcRect.y = 0;
    m_srcRect.w = 128;
    m_srcRect.h = 82;
    m_desRect.w = m_srcRect.w;
    m_desRect.h = m_srcRect.h;
    m_brunning = true;
    return true;
}

void App::update() { m_srcRect.x = 128 * int(((SDL_GetTicks() / 100) % 6)); }
void App::render() {
    SDL_RenderClear(m_pRenderer);
    SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255);
    SDL_RenderCopyEx(m_pRenderer, m_pTexture, &m_srcRect, &m_desRect, 0, 0, SDL_FLIP_VERTICAL);
    SDL_RenderPresent(m_pRenderer);
}

void App::clean() {
    std::cout << "cleaning game\n";
    SDL_DestroyWindow(m_pWindow);
    SDL_DestroyRenderer(m_pRenderer);
    SDL_Quit();
}
void App::handleEvents() {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                m_brunning = false;
                break;
            default:
                break;
        }
    }
}
