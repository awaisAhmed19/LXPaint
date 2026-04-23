#pragma once
#include <SDL3/SDL.h>

class Canvas {
   public:
    SDL_Renderer* renderer = NULL;
    SDL_Surface* drawingSurface = NULL;  // CPU-side: Where PutPixel happens
    SDL_Texture* mainTexture = NULL;     // GPU-side: The "mirror" of drawingSurface
    SDL_Texture* previewTexture = NULL;  // GPU-side: For live previews (Line/Rect)
    int w = 0, h = 0;
    SDL_Rect* gm_area = 0;
    Canvas(SDL_Renderer* r, int w, int h) : renderer(r) {
        this->w = w;
        this->h = h;
        drawingSurface = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_ARGB8888);

        // 2. Setup Hardware Mirror (Main) - STREAMING is for CPU->GPU updates
        mainTexture =
            SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);

        // 3. Setup Hardware Preview - TARGET is for Renderer->Texture updates
        previewTexture =
            SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);

        SDL_SetTextureBlendMode(previewTexture, SDL_BLENDMODE_BLEND);

        clearAll();
    }

    ~Canvas() {
        SDL_DestroySurface(drawingSurface);
        SDL_DestroyTexture(mainTexture);
        SDL_DestroyTexture(previewTexture);
    }
    // Instead of NULL (entire texture), pass an SDL_Rect of the modified area
    void syncArea(const SDL_Rect* area) {
        SDL_UpdateTexture(mainTexture, area, drawingSurface->pixels, drawingSurface->pitch);
    }
    // Push pixels from CPU Surface to GPU Texture
    void syncTexture() {
        SDL_UpdateTexture(mainTexture, NULL, drawingSurface->pixels, drawingSurface->pitch);
    }

    void clearPreview() {
        SDL_SetRenderTarget(renderer, previewTexture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);  // Fully Transparent
        SDL_RenderClear(renderer);
        SDL_SetRenderTarget(renderer, NULL);  // Reset target to screen
    }

    void clearAll() {
        SDL_FillSurfaceRect(drawingSurface, NULL, 0xFFFFFFFF);  // White background
        clearPreview();
        if (gm_area != 0) {
            syncArea(gm_area);
        } else {
            syncTexture();
        }
    }
};
