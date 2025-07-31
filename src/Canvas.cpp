#include "Canvas.h"

#include <algorithm>  // for std::fill

Canvas::Canvas(SDL_Renderer* ren, int x, int y, int w, int h)
    : renderer(ren), x(x), y(y), width(w), height(h) {
    texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!texture) {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
    }

    pitch = w * sizeof(uint32_t);
    pixelBuffer = new uint32_t[w * h];
    std::fill(pixelBuffer, pixelBuffer + w * h, 0xFFFFFFFF);  // Fill with white
}
Canvas::~Canvas() {
    delete[] pixelBuffer;
    SDL_DestroyTexture(texture);
}
void Canvas::render() {
    // Update texture with pixel buffer
    SDL_UpdateTexture(texture, nullptr, pixelBuffer, pitch);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);  // Render the whole canvas
}
