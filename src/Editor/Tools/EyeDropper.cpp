#include "EyeDropper.h"

#include "Editor/Interaction/ToolContext.h"
#include "Rendering/Rasterizer.h"
#include "Systems/Assert.h"
#include "Systems/Logger.h"

std::unique_ptr<Command> Eyedropper::onMouseClick(vec2 pos, ToolContext &ctx) {
  LX_ASSERT(ctx.canvas != nullptr, "Eyedropper canvas missing");

  SDL_Surface *surface = ctx.canvas->getSurface();

  int x = (int)pos.x;
  int y = (int)pos.y;

  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
    return nullptr;
  }

  // Sample directly from the canvas surface, never the preview layer.
  uint32_t *pixels = Rasterizer::getPixels(surface);
  int pitch = Rasterizer::getPitch(surface);
  uint32_t sampled = pixels[y * pitch + x];

  Logger::debug(
      std::format("Eyedropper sampled {:08X} at ({}, {})", sampled, x, y));

  if (m_rightClick) {
    if (ctx.bgColorOut)
      *ctx.bgColorOut = sampled;
  } else {
    if (ctx.fgColorOut)
      *ctx.fgColorOut = sampled;
  }

  // No canvas mutation occurred — no command, no undo entry.
  return nullptr;
}
