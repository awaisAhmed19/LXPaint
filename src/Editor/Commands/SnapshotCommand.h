#pragma once

#include "Command.h"
#include <SDL3/SDL.h>
#include <memory>

struct SDL_Surface_Deleter {
  void operator()(SDL_Surface *s) const {
    if (s)
      SDL_DestroySurface(s);
  }
};

using UniqueSurface = std::unique_ptr<SDL_Surface, SDL_Surface_Deleter>;

class SnapshotCommand : public Command {
private:
  SDL_Rect m_bounds{};

  UniqueSurface m_before;
  UniqueSurface m_after;

private:
  UniqueSurface copyRegion(SDL_Surface *src, SDL_Rect bounds) {
    SDL_Surface *copy = SDL_CreateSurface(bounds.w, bounds.h, src->format);

    SDL_Rect srcRect = bounds;

    SDL_BlitSurface(src, &srcRect, copy, nullptr);

    return UniqueSurface(copy);
  }

  void restoreRegion(SDL_Surface *dst, SDL_Surface *src, SDL_Rect bounds) {
    SDL_Rect dstRect = bounds;

    SDL_BlitSurface(src, nullptr, dst, &dstRect);
  }

public:
  SnapshotCommand(SDL_Surface *canvasSurface, SDL_Rect bounds)
      : m_bounds(bounds) {
    m_before = copyRegion(canvasSurface, bounds);
  }

  void captureAfter(SDL_Surface *canvasSurface) {
    m_after = copyRegion(canvasSurface, m_bounds);
  }

  void undo(Canvas &canvas) override {
    restoreRegion(canvas.getSurface(), m_before.get(), m_bounds);

    canvas.markDirty();
  }

  void redo(Canvas &canvas) override {
    restoreRegion(canvas.getSurface(), m_after.get(), m_bounds);

    canvas.markDirty();
  }

  size_t memoryUsage() const override {
    size_t before = m_before ? m_before->w * m_before->h * 4 : 0;

    size_t after = m_after ? m_after->w * m_after->h * 4 : 0;

    return before + after;
  }
};
