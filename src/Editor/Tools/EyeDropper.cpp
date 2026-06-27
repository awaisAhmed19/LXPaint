#include "EyeDropper.h"

#include "App/Globals.h"
#include "Systems/Logger.h"

std::unique_ptr<Command> Eyedropper::onMouseClick(vec2 pos, ToolContext &ctx) {
  if (!ctx.canvas || !ctx.fgColorOut)
    return nullptr;

  SDL_Surface *surface = ctx.canvas->getSurface();
  if (!surface)
    return nullptr;

  const int x = static_cast<int>(pos.x);
  const int y = static_cast<int>(pos.y);

  if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
    return nullptr;

  // Surface format is ARGB8888 — same layout as Editor's m_fgColor.
  if (!lockSurface(surface))
    return nullptr;

  const uint32_t *pixels = static_cast<const uint32_t *>(surface->pixels);
  const int pitch = surface->pitch / 4;
  const uint32_t sampled = pixels[y * pitch + x];

  unlockSurface(surface);

  // Write the sampled color into the editor's live foreground field.
  *ctx.fgColorOut = sampled;

  // Signal App::render() to push editor → palette this frame.
  if (ctx.colorSampledOut)
    *ctx.colorSampledOut = true;

  Logger::debug(
      std::format("Eyedropper sampled 0x{:08X} at ({},{})", sampled, x, y));

  // No canvas modification → no Command needed.
  return nullptr;
}
