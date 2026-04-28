#pragma once
#include "../Core/Canvas.h"  // Include this to see the members of Canvas
#include "../Core/Command.h" // MUST include the base class header
#include <SDL3/SDL.h>
#include <memory>

// Define a custom deleter for SDL surfaces
struct SDL_Surface_Deleter {
  void operator()(SDL_Surface *s) const {
    if (s)
      SDL_DestroySurface(s);
  }
};

using UniqueSurface = std::unique_ptr<SDL_Surface, SDL_Surface_Deleter>;

class DrawCommand : public Command {
private:
  UniqueSurface beforeSnapshot;
  UniqueSurface afterSnapshot;

public:
  DrawCommand(SDL_Surface *before, SDL_Surface *after) {
    beforeSnapshot.reset(SDL_DuplicateSurface(before));
    afterSnapshot.reset(SDL_DuplicateSurface(after));
  }

  // The 'override' works now because Command.h is included
  void execute(Canvas &canvas) override {
    if (afterSnapshot) {
      SDL_BlitSurface(afterSnapshot.get(), NULL, canvas.drawingSurface, NULL);
    }
  }

  void undo(Canvas &canvas) override {
    if (beforeSnapshot) {
      SDL_BlitSurface(beforeSnapshot.get(), NULL, canvas.drawingSurface, NULL);
    }
  }
};
