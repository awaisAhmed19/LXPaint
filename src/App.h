#pragma once
#include <SDL.h>
#include <SDL_image.h>
class App {
   public:
    App() {}

    ~App() {}

    bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
    void render();
    void update();
    void handleEvents();
    void clean();
    bool running() { return m_brunning; }

   private:
    SDL_Renderer* m_pRenderer = nullptr;
    SDL_Window* m_pWindow = nullptr;
    SDL_Texture* m_pTexture;
    SDL_Rect m_srcRect;
    SDL_Rect m_desRect;

    bool m_brunning;
};
