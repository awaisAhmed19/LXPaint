#pragma once
#include "../Core/Canvas.h"
#include "../Core/Command.h"
#include <SDL3/SDL.h>
#include <memory>

struct SDL_Surface_Deleter {
  void operator()(SDL_Surface *s) const {
    if (s)
      SDL_DestroySurface(s);
  }
};

using UniqueSurface = std::unique_ptr<SDL_Surface, SDL_Surface_Deleter>;

class DrawCommand : public Command {
private:
  SDL_Rect region{};
  UniqueSurface before;
  UniqueSurface after;

public:
  DrawCommand(SDL_Surface *canvasSurface, SDL_Rect dirtyRegion) {
    region = dirtyRegion;

    // Create BEFORE snapshot
    before.reset(SDL_CreateSurface(region.w, region.h, canvasSurface->format));

    SDL_BlitSurface(canvasSurface, &region, before.get(), NULL);
  }

  void captureAfter(SDL_Surface *canvasSurface) {
    // Create AFTER snapshot
    after.reset(SDL_CreateSurface(region.w, region.h, canvasSurface->format));

    SDL_BlitSurface(canvasSurface, &region, after.get(), NULL);
  }

  void execute(Canvas &canvas) override {
    if (after) {
      SDL_BlitSurface(after.get(), NULL, canvas.drawingSurface, &region);
    }
  }

  void undo(Canvas &canvas) override {
    if (before) {
      SDL_BlitSurface(before.get(), NULL, canvas.drawingSurface, &region);
    }
  }
};
