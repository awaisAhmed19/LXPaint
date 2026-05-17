#pragma once

#include "../../Document/Canvas.h"
#include "../../Systems/Assert.h"
#include "../../Systems/Logger.h"
#include "./Command.h"
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
  static void restoreRegion(SDL_Surface *dst, SDL_Surface *src,
                            SDL_Rect bounds) {
    LX_ASSERT(dst != nullptr, "restoreRegion dst null");
    LX_ASSERT(src != nullptr, "restoreRegion src null");
    Logger::debug(std::format("Snapshot restoreRegion x={} y={} w={} h={}",
                              bounds.x, bounds.y, bounds.w, bounds.h));

    SDL_Rect dstRect = bounds;

    if (!SDL_BlitSurface(src, nullptr, dst, &dstRect)) {

      Logger::log(LogLevel::ERR,
                  std::format("Restore blit failed: {}", SDL_GetError()));
    }
  }

public:
  /*
    GEOMETRIC TOOL CONSTRUCTOR

    Use for:
    - line
    - rect
    - ellipse

    where bounds are already known before drawing
  */
  SnapshotCommand(SDL_Surface *canvasSurface, SDL_Rect bounds)
      : m_bounds(bounds) {

    Logger::debug("SnapshotCommand geometric ctor");

    m_before = copyRegion(canvasSurface, bounds);
  }

  /*
    STROKE TOOL CONSTRUCTOR

    Use for:
    - pencil
    - eraser
    - brush

    where final bounds are known only after stroke ends
  */
  SnapshotCommand(UniqueSurface before, UniqueSurface after, SDL_Rect bounds)
      : m_bounds(bounds), m_before(std::move(before)),
        m_after(std::move(after)) {

    Logger::debug("SnapshotCommand stroke ctor");
  }

  void captureAfter(SDL_Surface *canvasSurface) {

    Logger::debug("Snapshot captureAfter");

    m_after = copyRegion(canvasSurface, m_bounds);
  }

  void undo(Canvas &canvas) override {
    LX_ASSERT(m_before != nullptr, "Undo snapshot missing");
    // if (!m_before) {
    //   Logger::log(LogLevel::ERR, "Undo failed: m_before missing");
    //   return;
    // }
    restoreRegion(canvas.getSurface(), m_before.get(), m_bounds);
    canvas.markDirty();
  }

  void redo(Canvas &canvas) override {
    LX_ASSERT(m_after != nullptr, "Redo snapshot missing");
    restoreRegion(canvas.getSurface(), m_after.get(), m_bounds);
    canvas.markDirty();
  }
  static UniqueSurface copyRegion(SDL_Surface *src, SDL_Rect bounds) {
    LX_ASSERT(src != nullptr, "copyRegion source null");
    LX_ASSERT(bounds.w > 0, "Invalid snapshot width");
    LX_ASSERT(bounds.h > 0, "Invalid snapshot height");

    Logger::debug(std::format("Snapshot copyRegion x={} y={} w={} h={}",
                              bounds.x, bounds.y, bounds.w, bounds.h));

    SDL_Surface *copy = SDL_CreateSurface(bounds.w, bounds.h, src->format);

    if (!copy) {
      Logger::log(LogLevel::ERR,
                  std::format("SDL_CreateSurface failed: {}", SDL_GetError()));

      return nullptr;
    }

    SDL_Rect srcRect = bounds;

    if (!SDL_BlitSurface(src, &srcRect, copy, nullptr)) {

      Logger::log(LogLevel::ERR,
                  std::format("SDL_BlitSurface failed: {}", SDL_GetError()));
    }

    return UniqueSurface(copy);
  }

  size_t memoryUsage() const override {

    size_t before = m_before ? m_before->w * m_before->h * 4 : 0;
    size_t after = m_after ? m_after->w * m_after->h * 4 : 0;

    return before + after;
  }
};
