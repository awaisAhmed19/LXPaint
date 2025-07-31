#pragma once
#include <SDL2/SDL.h>
class Canvas {
   private:
    int x, y;
    int width, height;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    SDL_Color currColor;
    int brushSize;

    uint32_t* pixelBuffer;
    int pitch;

   public:
    Canvas(SDL_Renderer* ren, int x, int y, int w, int h);
    ~Canvas();
    void drawPoint(int MouseX, int MouseY);
    void erasePoint(int MouseX, int MouseY);
    void clear();

    void handleEvent(SDL_Event& e);

    void render();
};
